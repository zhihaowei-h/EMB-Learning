// 1.3.9 IO多路转接需求
// src/io_multiplexing.c
#include "io_multiplexing.h"
#include "ipc_modules.h"
#include "password_auth.h" 
#include "token_bucket.h" 
#include "process_pool.h"  // [新增] 引入进程池模块

static int g_client_auth_state[65536] = {0}; 

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int init_tcp_server(int port) {
    int listen_fd;
    struct sockaddr_in server_addr;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_FATAL("TCP Socket creation failed: %s", strerror(errno));
        return -1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(listen_fd);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        LOG_FATAL("TCP Bind failed on port %d: %s", port, strerror(errno));
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        LOG_FATAL("TCP Listen failed: %s", strerror(errno));
        close(listen_fd);
        return -1;
    }

    LOG_INFO("TCP server listening on port %d", port);
    return listen_fd;
}

int init_udp_server(int port) {
    int udp_fd;
    struct sockaddr_in server_addr;

    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LOG_FATAL("UDP Socket creation failed: %s", strerror(errno));
        return -1;
    }

    set_nonblocking(udp_fd);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(udp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        LOG_FATAL("UDP Bind failed on port %d: %s", port, strerror(errno));
        close(udp_fd);
        return -1;
    }

    LOG_INFO("UDP server listening on port %d", port);
    return udp_fd;
}

typedef struct {
    char data[BUFFER_SIZE];
    ssize_t len;
    struct sockaddr_in client_addr;
} UdpPacketArgs;

static void handle_udp_packet(void *arg) {
    UdpPacketArgs *packet = (UdpPacketArgs *)arg;
    packet->data[packet->len] = '\0';
    packet->data[strcspn(packet->data, "\r\n")] = 0; 
    
    LOG_INFO("UDP RECV -> %s", packet->data);
    update_sensor_data_cache(packet->data);
    free(packet);
}

static void handle_tcp_client_data(void *arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (n > 0) {
        buffer[n] = '\0';
        buffer[strcspn(buffer, "\r\n")] = 0; 
        char response[BUFFER_SIZE + 256];

        if (g_client_auth_state[client_fd] == 0) {
            char user[64], pass[64];
            if (sscanf(buffer, "AUTH %63s %63s", user, pass) == 2) {
                if (authenticate_user(user, pass)) {
                    g_client_auth_state[client_fd] = 1; 
                    snprintf(response, sizeof(response), "AUTH_OK\n");
                } else {
                    snprintf(response, sizeof(response), "AUTH_FAIL\n");
                    send(client_fd, response, strlen(response), 0);
                    close(client_fd); 
                    return;
                }
            } else {
                snprintf(response, sizeof(response), "ACCESS DENIED. Usage: AUTH <username> <password>\n");
            }
            send(client_fd, response, strlen(response), 0);
            return; 
        }

        // --- 业务路由 ---
        if (strcmp(buffer, "GET_SENSOR") == 0) {
            SensorDataCache cache;
            read_sensor_data_cache(&cache);
            snprintf(response, sizeof(response), 
                     "LATEST SENSOR: %s (Packets: %d, Time: %ld)\n", 
                     cache.latest_sensor_data, cache.total_packets_received, cache.last_update_time);
        } 
        // [新增] 进程池路由分配
        else if (strncmp(buffer, "PLUGIN ", 7) == 0) {
            send_to_plugin_fifo(buffer + 7); // 把任务内容压入 FIFO 管道
            snprintf(response, sizeof(response), "Task dispatched to isolated Process Pool.\n");
        } 
        else {
            snprintf(response, sizeof(response), "ECHO: %s\n", buffer);
        }
        
        send(client_fd, response, strlen(response), 0);
        
    } else if (n == 0) {
        LOG_INFO("TCP FD %d disconnected.", client_fd);
        g_client_auth_state[client_fd] = 0; 
        close(client_fd);
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("TCP Recv error on FD %d: %s", client_fd, strerror(errno));
            g_client_auth_state[client_fd] = 0; 
            close(client_fd);
        }
    }
}

void start_event_loop(ThreadPool *pool, int sig_pipe_read_fd) {
    int tcp_listen_fd = init_tcp_server(TCP_PORT);
    int udp_listen_fd = init_udp_server(UDP_PORT); 
    
    if (tcp_listen_fd < 0 || udp_listen_fd < 0) exit(1);

    TokenBucket *udp_bucket = token_bucket_create(5, 2);

    struct pollfd *fds = calloc(MAX_TCP_CONNECTIONS + 3, sizeof(struct pollfd));
    int nfds = 3; 

    fds[0].fd = sig_pipe_read_fd; fds[0].events = POLLIN;
    fds[1].fd = tcp_listen_fd;    fds[1].events = POLLIN;
    fds[2].fd = udp_listen_fd;    fds[2].events = POLLIN;

    for (int i = 3; i < MAX_TCP_CONNECTIONS + 3; i++) { fds[i].fd = -1; }

    LOG_INFO("Event loop started. Multiplexing TCP, UDP and Signals...");

    while (g_keep_running) {
        int ret = poll(fds, nfds, -1); 

        if (ret < 0) {
            if (errno == EINTR) continue; 
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (fds[i].fd == -1) continue;
            
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                if (fds[i].fd != sig_pipe_read_fd && fds[i].fd != tcp_listen_fd && fds[i].fd != udp_listen_fd) {
                    LOG_WARN("FD %d exception/disconnected", fds[i].fd);
                    g_client_auth_state[fds[i].fd] = 0; 
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
                continue;
            }

            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == sig_pipe_read_fd) {
                    char sig_byte; read(sig_pipe_read_fd, &sig_byte, 1);
                    g_keep_running = 0; 
                    break; 
                } 
                else if (fds[i].fd == tcp_listen_fd) {
                    struct sockaddr_in client_addr; socklen_t client_len = sizeof(client_addr);
                    int new_fd = accept(tcp_listen_fd, (struct sockaddr*)&client_addr, &client_len);
                    
                    if (new_fd >= 0) {
                        set_nonblocking(new_fd); 
                        g_client_auth_state[new_fd] = 0; 

                        int j;
                        for (j = 3; j < MAX_TCP_CONNECTIONS + 3; j++) {
                            if (fds[j].fd == -1) {
                                fds[j].fd = new_fd; fds[j].events = POLLIN;
                                if (j >= nfds) nfds = j + 1;
                                break;
                            }
                        }
                        if (j == MAX_TCP_CONNECTIONS + 3) close(new_fd);
                    }
                } 
                else if (fds[i].fd == udp_listen_fd) {
                    if (token_bucket_consume(udp_bucket, 1)) {
                        UdpPacketArgs *packet = malloc(sizeof(UdpPacketArgs));
                        socklen_t addr_len = sizeof(packet->client_addr);
                        packet->len = recvfrom(udp_listen_fd, packet->data, BUFFER_SIZE - 1, 0, (struct sockaddr*)&(packet->client_addr), &addr_len);
                        if (packet->len > 0) { thread_pool_add_task(pool, handle_udp_packet, packet); } 
                        else { free(packet); }
                    } else {
                        char dump_buf[256];
                        recvfrom(udp_listen_fd, dump_buf, sizeof(dump_buf), 0, NULL, NULL);
                        LOG_WARN("UDP rate limit exceeded, dropping packet");
                    }
                }
                else {
                    int *task_fd = malloc(sizeof(int)); *task_fd = fds[i].fd;
                    thread_pool_add_task(pool, handle_tcp_client_data, task_fd);
                }
            }
        }
    }
    free(fds); close(tcp_listen_fd); close(udp_listen_fd);
    token_bucket_destroy(udp_bucket);
}