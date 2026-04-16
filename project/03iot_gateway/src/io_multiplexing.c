/*===============================================
 * 文件名称：io_multiplexing.c
 * 创 建 者：IoT Gateway System
 * 创建日期：2026年04月15日
 * 描    述：IO多路复用主循环
 * 基于poll实现高并发TCP/UDP服务器
 ================================================*/

#include "iot_gateway.h"
#include "cloud_upload.h"

// 全局网络相关变量
int g_tcp_sockfd = -1;
int g_udp_sockfd = -1;
int g_cloud_sockfd = -1;
int g_running = 1;
int g_signal_pipe[2] = {-1, -1};

// 移除 static，使其变为全局变量，供 main.c 使用
int g_client_count = 0;

// poll监听数组
static struct pollfd g_poll_fds[MAX_POLL_FDS];
static int g_nfds = 0;

// TCP客户端连接数组
static tcp_client_t g_clients[MAX_CLIENTS];


/*
 * 功能：设置套接字为非阻塞模式
 */
void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/*
 * 功能：获取当前时间戳字符串
 */
const char *get_timestamp(void)
{
    static char timestamp[32];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    return timestamp;
}

/*
 * 功能：创建TCP服务器套接字
 */
int create_tcp_server(int port)
{
    int sockfd;
    struct sockaddr_in addr;
    int opt = 1;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(sockfd);
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }
    
    if (listen(sockfd, 128) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }
    
    log_write(LOG_INFO, "TCP server listening on port %d", port);
    return sockfd;
}

/*
 * 功能：创建UDP服务器套接字
 */
int create_udp_server(int port)
{
    int sockfd;
    struct sockaddr_in addr;
    int opt = 1;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(sockfd);
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }
    
    log_write(LOG_INFO, "UDP server listening on port %d", port);
    return sockfd;
}

/*
 * 功能：处理TCP新连接
 */
void handle_tcp_accept(int listen_fd)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    while ((client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len)) > 0) {
        if (g_client_count >= MAX_CLIENTS) {
            log_write(LOG_WARN, "Max clients reached, rejecting connection");
            close(client_fd);
            continue;
        }
        
        set_nonblocking(client_fd);
        
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (g_clients[i].fd == -1) {
                g_clients[i].fd = client_fd;
                g_clients[i].addr = client_addr;
                g_clients[i].last_active = time(NULL);
                g_clients[i].authenticated = 0;
                g_clients[i].recv_len = 0;
                
                g_poll_fds[g_nfds].fd = client_fd;
                g_poll_fds[g_nfds].events = POLLIN;
                g_nfds++;
                
                g_client_count++;
                
                log_write(LOG_INFO, "New client connected: %s:%d (fd=%d, total=%d)",
                          inet_ntoa(client_addr.sin_addr),
                          ntohs(client_addr.sin_port),
                          client_fd, g_client_count);
                break;
            }
        }
    }
}

/*
 * 功能：处理TCP客户端数据
 */
void handle_tcp_client(tcp_client_t *client)
{
    char buf[BUFFER_SIZE];
    ssize_t n;
    
    n = recv(client->fd, buf, sizeof(buf) - 1, 0);
    
    if (n <= 0) {
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return;
        }
        
        log_write(LOG_INFO, "Client disconnected: fd=%d", client->fd);
        
        int i;
        for (i = 0; i < g_nfds; i++) {
            if (g_poll_fds[i].fd == client->fd) {
                g_poll_fds[i] = g_poll_fds[g_nfds - 1];
                g_nfds--;
                break;
            }
        }
        
        close(client->fd);
        client->fd = -1;
        g_client_count--;
        return;
    }
    
    buf[n] = '\0';
    client->last_active = time(NULL);
    
    if (!client->authenticated) {
        if (token_verify(buf) == 0) {
            client->authenticated = 1;
            const char *reply = "AUTH_OK\n";
            send(client->fd, reply, strlen(reply), 0);
            log_write(LOG_INFO, "Client authenticated: fd=%d", client->fd);
        } else {
            const char *reply = "AUTH_FAILED\n";
            send(client->fd, reply, strlen(reply), 0);
            close(client->fd);
            client->fd = -1;
            g_client_count--;
        }
        return;
    }
    
    log_write(LOG_DEBUG, "Received from client fd=%d: %s", client->fd, buf);
    
    char reply[BUFFER_SIZE];
    snprintf(reply, sizeof(reply), "ECHO: %s", buf);
    send(client->fd, reply, strlen(reply), 0);
}

/*
 * 功能：解析STM32传感器数据
 */
static int parse_sensor_data(const char *raw_data, sensor_data_t *sensor)
{
    int year, month, day, week, hour, min, sec;
    int dht_temp, dht_hum, adj;
    double sht_temp, sht_hum;
    
    int ret = sscanf(raw_data, "%d-%d-%d %d %d:%d:%d DHT:%d/%d SHT:%lf/%lf ADJ:%d",
                     &year, &month, &day, &week, &hour, &min, &sec,
                     &dht_temp, &dht_hum, &sht_temp, &sht_hum, &adj);
    
    if (ret == 12) {
        sensor->sensor_id = 1;  
        sensor->temperature = sht_temp;
        sensor->humidity = sht_hum;
        sensor->adj_value = adj;
        snprintf(sensor->timestamp, sizeof(sensor->timestamp),
                 "%04d-%02d-%02d %02d:%02d:%02d",
                 year, month, day, hour, min, sec);
        strncpy(sensor->raw_data, raw_data, sizeof(sensor->raw_data) - 1);
        return 0;
    }
    return -1;
}

/*
 * 【新增】：异步上传任务的包装函数（供线程池调用）
 */
static void async_upload_task(void *arg)
{
    sensor_data_t *data = (sensor_data_t *)arg;
    upload_to_cloud(data);
    free(data); // 必须释放动态分配的内存
}

/*
 * 功能：处理UDP数据
 */
void handle_udp_data(void)
{
    char buf[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    ssize_t n;
    
    if (token_bucket_fetch(g_token_bucket, 1) == 0) {
        log_write(LOG_WARN, "UDP rate limit exceeded, dropping packet");
        return;
    }
    
    n = recvfrom(g_udp_sockfd, buf, sizeof(buf) - 1, 0,
                 (struct sockaddr *)&client_addr, &addr_len);
    
    if (n <= 0) return;
    
    buf[n] = '\0';
    
    log_write(LOG_INFO, "Received UDP from %s:%d: %s",
              inet_ntoa(client_addr.sin_addr),
              ntohs(client_addr.sin_port), buf);
    
    sensor_data_t sensor;
    if (parse_sensor_data(buf, &sensor) == 0) {
        
        sem_p(g_semid, 0);
        if (g_shm_ptr->sensor_count < SENSOR_COUNT) {
            g_shm_ptr->sensors[g_shm_ptr->sensor_count] = sensor;
            g_shm_ptr->sensor_count++;
        } else {
            g_shm_ptr->sensors[0] = sensor;
        }
        g_shm_ptr->last_update = time(NULL);
        sem_v(g_semid, 0);
        
        log_write(LOG_INFO, "Sensor data updated: temp=%.2f, hum=%.2f, adj=%d",
                  sensor.temperature, sensor.humidity, sensor.adj_value);
        
        // 【修改点】：不再直接调用 upload_to_cloud，而是加入线程池异步执行
        sensor_data_t *task_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
        if (task_data != NULL) {
            memcpy(task_data, &sensor, sizeof(sensor_data_t));
            if (thread_pool_add_task(g_thread_pool, async_upload_task, task_data) < 0) {
                log_write(LOG_ERROR, "Failed to add upload task to thread pool");
                free(task_data);
            }
        } else {
            log_write(LOG_ERROR, "Failed to allocate memory for async upload task");
        }
    }
}

/*
 * 功能：上传数据到云平台
 */
void upload_to_cloud(sensor_data_t *data)
{
    if (data == NULL) return;
    
    cloud_upload_single_data(data->sensor_id, data->temperature);
    
    log_write(LOG_INFO, "Data uploaded to cloud via HTTP: sensor=%d, temp=%.2f, hum=%.2f",
              data->sensor_id, data->temperature, data->humidity);
}

/*
 * 功能：信号处理函数
 */
void signal_handler(int signo)
{
    char sig = (char)signo;
    if (write(g_signal_pipe[1], &sig, 1) < 0) {
        // 忽略警告
    }
}

/*
 * 功能：设置信号处理函数
 */
void setup_signal_handlers(void)
{
    struct sigaction sa;
    
    if (pipe(g_signal_pipe) < 0) {
        perror("pipe");
        exit(1);
    }
    
    set_nonblocking(g_signal_pipe[0]);
    set_nonblocking(g_signal_pipe[1]);
    
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    
    signal(SIGPIPE, SIG_IGN);
    
    log_write(LOG_INFO, "Signal handlers initialized");
}

/*
 * 功能：初始化IO系统
 */
int io_init(void)
{
    int i;
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        g_clients[i].fd = -1;
    }
    
    g_tcp_sockfd = create_tcp_server(g_config.tcp_port);
    if (g_tcp_sockfd < 0) return -1;
    
    g_udp_sockfd = create_udp_server(g_config.udp_port);
    if (g_udp_sockfd < 0) {
        close(g_tcp_sockfd);
        return -1;
    }
    
    g_cloud_sockfd = -1;
    g_nfds = 0;
    
    g_poll_fds[g_nfds].fd = g_tcp_sockfd;
    g_poll_fds[g_nfds].events = POLLIN;
    g_nfds++;
    
    g_poll_fds[g_nfds].fd = g_udp_sockfd;
    g_poll_fds[g_nfds].events = POLLIN;
    g_nfds++;
    
    g_poll_fds[g_nfds].fd = g_signal_pipe[0];
    g_poll_fds[g_nfds].events = POLLIN;
    g_nfds++;
    
    log_write(LOG_INFO, "IO system initialized");
    return 0;
}

/*
 * 功能：IO主循环
 */
void io_loop(void)
{
    int i, ret;
    char sig;
    
    log_write(LOG_INFO, "Entering main event loop");
    
    while (g_running) {
        ret = poll(g_poll_fds, g_nfds, 1000);
        
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        
        if (ret == 0) continue;
        
        for (i = 0; i < g_nfds; i++) {
            if (g_poll_fds[i].revents == 0) continue;
            
            if (g_poll_fds[i].fd == g_tcp_sockfd && (g_poll_fds[i].revents & POLLIN)) {
                handle_tcp_accept(g_tcp_sockfd);
            }
            else if (g_poll_fds[i].fd == g_udp_sockfd && (g_poll_fds[i].revents & POLLIN)) {
                handle_udp_data();
            }
            else if (g_poll_fds[i].fd == g_signal_pipe[0] && (g_poll_fds[i].revents & POLLIN)) {
                if (read(g_signal_pipe[0], &sig, 1) > 0) {
                    log_write(LOG_INFO, "Received signal: %d", sig);
                    if (sig == SIGTERM || sig == SIGINT) {
                        log_write(LOG_INFO, "Shutting down gracefully...");
                        g_running = 0;
                    }
                }
            }
            else {
                int j;
                for (j = 0; j < MAX_CLIENTS; j++) {
                    if (g_clients[j].fd == g_poll_fds[i].fd) {
                        handle_tcp_client(&g_clients[j]);
                        break;
                    }
                }
            }
        }
    }
    
    log_write(LOG_INFO, "Exiting main event loop");
}

/*
 * 功能：清理IO资源
 */
void io_cleanup(void)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].fd >= 0) close(g_clients[i].fd);
    }
    if (g_tcp_sockfd >= 0) close(g_tcp_sockfd);
    if (g_udp_sockfd >= 0) close(g_udp_sockfd);
    if (g_cloud_sockfd >= 0) close(g_cloud_sockfd);
    if (g_signal_pipe[0] >= 0) close(g_signal_pipe[0]);
    if (g_signal_pipe[1] >= 0) close(g_signal_pipe[1]);
    
    log_write(LOG_INFO, "IO resources cleaned up");
}