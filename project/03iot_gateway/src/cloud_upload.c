#include "../inc/cloud_upload.h"
#include "../inc/base64.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/*
 * 功能：创建TCP连接
 */
static int cloud_tcp_connect(const char *ip, int port)
{
    int sockfd;
    struct sockaddr_in server_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    struct timeval tv = {5, 0}; 
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

/*
 * 功能：上传单个传感器数据
 * 重点：将 device/%d 替换为节点 ID 5951
 */
int cloud_upload_single_data(int sensor_id, double data_value)
{
    (void)sensor_id; 
    int sockfd;
    int ret;
    char send_data[1024] = {0};
    char body[128] = {0};
    char base_id[256] = {0};
    char response[1024] = {0};

    // 1. 构造 JSON 正文
    int body_len = sprintf(body, "{\"data\":%.2f}", data_value);

    // 2. 身份认证 Base64 编码
    base64_encode_string((unsigned char *)base_id, 
                        (const unsigned char *)g_config.api_key);

    // 3. 构造报文：关键在于路径中的 5951 和 16840
    ret = sprintf(send_data,
        "POST /api/1.0/device/%d/sensor/%d/data HTTP/1.1\r\n"
        "Host: www.embsky.com\r\n"
        "Authorization: Basic %s\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        5951,   // <--- 这里改成了节点编号 5951
        16840,  // <--- 传感器编号 16840
        base_id,
        body_len,
        body);

    log_write(LOG_DEBUG, "Sending to Node 5951, Sensor 16840...");

    sockfd = cloud_tcp_connect(g_config.cloud_server, 80);
    if (sockfd < 0) return -1;

    if (send(sockfd, send_data, ret, 0) < 0) {
        close(sockfd);
        return -1;
    }

    ret = recv(sockfd, response, sizeof(response) - 1, 0);
    if (ret > 0 && strstr(response, "200 OK")) {
        log_write(LOG_INFO, "Cloud upload SUCCESS for Node 5951: %.2f", data_value);
    }

    close(sockfd);
    return 0;
}

/*
 * 功能：多传感器上传
 */
int cloud_upload_sensor_datas(sensor_data_t *data, int count)
{
    if (data == NULL || count <= 0) return -1;
    int sockfd, ret, i;
    char send_data[2048] = {0}, data_list[1024] = {0}, body[1100] = {0}, base_id[256] = {0};

    int data_list_len = sprintf(data_list, "[");
    for (i = 0; i < count; i++) {
        data_list_len += sprintf(data_list + data_list_len, 
            "{\"id\":16840, \"data\":%.2f}%s", 
            data[i].temperature, (i == count - 1) ? "" : ",");
    }
    sprintf(data_list + data_list_len, "]");

    int body_len = sprintf(body, "{\"datas\":%s}", data_list);
    base64_encode_string((unsigned char *)base_id, (const unsigned char *)g_config.api_key);
    
    ret = sprintf(send_data, 
        "POST /api/1.0/device/%d/datas HTTP/1.1\r\n"
        "Host: www.embsky.com\r\n"
        "Authorization: Basic %s\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        5951, // <--- 批量接口也改为 5951
        base_id, body_len, body);
    
    sockfd = cloud_tcp_connect(g_config.cloud_server, 80);
    if (sockfd < 0) return -1;
    send(sockfd, send_data, ret, 0);
    close(sockfd);
    return 0;
}