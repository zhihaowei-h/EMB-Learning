#ifndef IOT_GATEWAY_H
#define IOT_GATEWAY_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>

// --- 系统宏定义 ---
#define MAX_TCP_CONNECTIONS 5000   // 支持的最大TCP连接数 [cite: 6]
#define TCP_PORT 8080
#define UDP_PORT 8081
#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define PID_FILE "/tmp/iot_gateway.pid"

// --- 日志级别定义 --- [cite: 46]
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

// --- 核心结构体前置声明 ---
// 客户端连接上下文
typedef struct {
    int fd;
    char ip[INET_ADDRSTRLEN];
    int is_authenticated;
    time_t last_heartbeat;
} ClientContext;

// 配置文件结构
typedef struct {
    int tcp_port;
    int udp_port;
    LogLevel log_level;
    char log_path[256];
    int thread_pool_size;
} AppConfig;

// --- 全局变量声明 ---
extern AppConfig g_config;
extern volatile sig_atomic_t g_keep_running; // 优雅退出标志 [cite: 72]

// --- 核心函数原型声明 ---
// (后续模块实现时逐步添加)
// --- 日志宏定义 (自动传入文件名和行号) ---
#define LOG_DEBUG(fmt, ...) gateway_log(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  gateway_log(LOG_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  gateway_log(LOG_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) gateway_log(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) gateway_log(LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// --- 函数声明 ---
void gateway_log(LogLevel level, const char *file, int line, const char *format, ...);
void daemonize();
int already_running();


#endif // IOT_GATEWAY_H