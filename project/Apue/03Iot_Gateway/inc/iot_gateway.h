/*===============================================
 *   文件名称：iot_gateway.h
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：物联网网关全局头文件
 *            定义所有数据结构、宏、外部声明
 ================================================*/

#ifndef __IOT_GATEWAY_H
#define __IOT_GATEWAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <shadow.h>
#include <crypt.h>

/*==========================
 * 全局宏定义
 *==========================*/
#define MAX_CLIENTS         5000        // 最大TCP客户端连接数
#define MAX_POLL_FDS        (MAX_CLIENTS + 10)  // poll监听的最大fd数
#define BUFFER_SIZE         4096        // 缓冲区大小
#define LOG_PATH_SIZE       256         // 日志路径长度
#define CONFIG_LINE_SIZE    512         // 配置文件行长度
#define PID_FILE            "/var/run/iot_gateway.pid"  // PID文件路径
#define FIFO_PATH           "/tmp/iot_fifo"  // 命名管道路径

// 传感器数据相关
#define SENSOR_DATA_SIZE    128         // 传感器数据大小
#define SENSOR_COUNT        10          // 最大传感器数量

// 云平台相关
#define CLOUD_SERVER        "119.29.98.16"  // 云平台IP
#define CLOUD_PORT          5924        // 云平台端口
#define API_KEY_SIZE        256         // API密钥长度

// IPC相关
#define SHM_KEY_PATH        "/tmp"      // 共享内存key路径
#define SHM_KEY_ID          'S'         // 共享内存key ID
#define MSG_KEY_ID          'M'         // 消息队列key ID
#define SEM_KEY_ID          'E'         // 信号量key ID

/*==========================
 * 日志级别定义
 *==========================*/
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

/*==========================
 * 配置结构体
 *==========================*/
typedef struct {
    int tcp_port;               // TCP监听端口
    int udp_port;               // UDP监听端口
    log_level_t log_level;      // 日志级别
    char log_path[LOG_PATH_SIZE];  // 日志路径
    int thread_pool_size;       // 线程池大小
    int process_pool_size;      // 进程池大小
    int token_cps;              // 令牌桶速率(每秒)
    int token_burst;            // 令牌桶容量
    char auth_token[256];       // 认证令牌
    char api_key[API_KEY_SIZE]; // 云平台API密钥
    char cloud_server[64];      // 云平台服务器地址
    int cloud_port;             // 云平台端口（这里指设备编号，如5924）
    int cloud_http_port;        // 云平台HTTP端口（固定80）
} config_t;

/*==========================
 * 传感器数据结构体
 *==========================*/
typedef struct {
    int sensor_id;              // 传感器ID
    double temperature;         // 温度
    double humidity;            // 湿度
    int adj_value;              // 可调电阻值
    char timestamp[32];         // 时间戳
    char raw_data[SENSOR_DATA_SIZE];  // 原始数据
} sensor_data_t;

/*==========================
 * 共享内存数据结构
 *==========================*/
typedef struct {
    int sensor_count;           // 当前传感器数量
    sensor_data_t sensors[SENSOR_COUNT];  // 传感器数组
    time_t last_update;         // 最后更新时间
} shm_data_t;

/*==========================
 * 消息队列消息结构
 *==========================*/
typedef struct {
    long msg_type;              // 消息类型
    char msg_text[BUFFER_SIZE]; // 消息内容
} msg_data_t;

/*==========================
 * TCP客户端连接结构体
 *==========================*/
typedef struct {
    int fd;                     // 套接字描述符
    struct sockaddr_in addr;    // 客户端地址
    time_t last_active;         // 最后活跃时间
    int authenticated;          // 是否已认证
    char recv_buf[BUFFER_SIZE]; // 接收缓冲区
    int recv_len;               // 接收数据长度
} tcp_client_t;

/*==========================
 * 线程池任务结构体
 *==========================*/
typedef struct task_node {
    void (*function)(void *arg);  // 任务函数指针
    void *arg;                   // 任务参数
    struct task_node *next;      // 下一个任务节点
} task_t;

/*==========================
 * 线程池结构体
 *==========================*/
typedef struct {
    pthread_t *threads;          // 线程数组
    int thread_count;            // 线程数量
    task_t *task_queue_head;     // 任务队列头
    task_t *task_queue_tail;     // 任务队列尾
    int task_count;              // 当前任务数量
    int shutdown;                // 关闭标志
    pthread_mutex_t lock;        // 互斥锁
    pthread_cond_t notify;       // 条件变量
} thread_pool_t;

/*==========================
 * 进程池子进程结构体
 *==========================*/
typedef struct {
    pid_t pid;                   // 子进程PID
    int pipe_fd[2];              // 与子进程通信的管道
    int busy;                    // 是否忙碌
} process_t;

/*==========================
 * 进程池结构体
 *==========================*/
typedef struct {
    process_t *processes;        // 进程数组
    int process_count;           // 进程数量
} process_pool_t;

/*==========================
 * 令牌桶结构体
 *==========================*/
typedef struct {
    int token;                   // 当前令牌数
    int cps;                     // 每秒产生的令牌数
    int burst;                   // 令牌桶容量
    pthread_mutex_t lock;        // 互斥锁
} token_bucket_t;

/*==========================
 * 定时任务回调函数类型
 *==========================*/
typedef void (*alarm_callback_t)(void);

/*==========================
 * 定时任务结构体
 *==========================*/
typedef struct alarm_task {
    int interval;                // 时间间隔(秒)
    int countdown;               // 倒计时
    alarm_callback_t callback;   // 回调函数
    struct alarm_task *next;     // 下一个任务
} alarm_task_t;

/*==========================
 * 全局变量声明
 *==========================*/
extern config_t g_config;        // 全局配置
extern int g_running;            // 运行标志
extern int g_signal_pipe[2];    // 信号管道
extern int g_client_count;       // 添加这一行：当前TCP客户端数量

// IPC资源ID
extern int g_msgid;              // 消息队列ID
extern int g_shmid;              // 共享内存ID
extern int g_semid;              // 信号量ID
extern shm_data_t *g_shm_ptr;    // 共享内存指针

// 网络相关
extern int g_tcp_sockfd;         // TCP监听套接字
extern int g_udp_sockfd;         // UDP套接字
extern int g_cloud_sockfd;       // 云平台连接套接字

// 线程池和进程池
extern thread_pool_t *g_thread_pool;   // 线程池
extern process_pool_t *g_process_pool; // 进程池

// 令牌桶
extern token_bucket_t *g_token_bucket; // 令牌桶

/*==========================
 * 函数声明
 *==========================*/

// daemon.c - 守护进程化
int daemonize(void);
int create_pid_file(void);
void remove_pid_file(void);

// log.c - 日志模块
void log_init(const char *log_path, log_level_t level);
void log_write(log_level_t level, const char *fmt, ...);
void log_close(void);

// config.c - 配置模块
int config_load(const char *config_file);
void config_print(void);

// thread_pool.c - 线程池
thread_pool_t *thread_pool_create(int thread_count);
int thread_pool_add_task(thread_pool_t *pool, void (*function)(void *), void *arg);
void thread_pool_destroy(thread_pool_t *pool);

// process_pool.c - 进程池
process_pool_t *process_pool_create(int process_count);
int process_pool_execute(process_pool_t *pool, const char *cmd);
void process_pool_destroy(process_pool_t *pool);

// token_bucket.c - 令牌桶
token_bucket_t *token_bucket_create(int cps, int burst);
int token_bucket_fetch(token_bucket_t *tb, int n);
void token_bucket_destroy(token_bucket_t *tb);
void token_bucket_refill(void);

// alarm_scheduler.c - 闹钟调度器
void alarm_scheduler_init(void);
int alarm_scheduler_add(int interval, alarm_callback_t callback);
void alarm_scheduler_run(void);
void alarm_scheduler_destroy(void);

// ipc_modules.c - IPC模块
int ipc_init(void);
void ipc_cleanup(void);
int sem_p(int semid, int index);
int sem_v(int semid, int index);

// password_auth.c - 密码认证
int password_verify(const char *username, const char *password);
int token_verify(const char *token);

// io_multiplexing.c - IO多路复用
int io_init(void);
void io_loop(void);
void io_cleanup(void);

// 网络相关函数
int create_tcp_server(int port);
int create_udp_server(int port);
int connect_to_cloud(const char *server, int port);
void handle_tcp_accept(int listen_fd);
void handle_tcp_client(tcp_client_t *client);
void handle_udp_data(void);
void upload_to_cloud(sensor_data_t *data);

// 信号处理
void setup_signal_handlers(void);
void signal_handler(int signo);

// 工具函数
void set_nonblocking(int fd);
const char *get_timestamp(void);

#endif /* __IOT_GATEWAY_H */
