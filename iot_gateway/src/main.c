// src/main.c
#include "iot_gateway.h"
#include "thread_pool.h"
#include "io_multiplexing.h"
#include "ipc_modules.h"
#include "process_pool.h"

AppConfig g_config;                       // 全局配置变量
volatile sig_atomic_t g_keep_running = 1; // 优雅退出标志
int g_sig_pipe[2]; 

void sig_handler(int signo) {
    int save_errno = errno; 
    char sig_byte = (signo == SIGINT) ? 'I' : 'T'; 
    write(g_sig_pipe[1], &sig_byte, 1);
    errno = save_errno;
}

int main(int argc, char *argv[]) {
    g_config.log_level = LOG_DEBUG;
    g_config.tcp_port = TCP_PORT; 
    g_config.udp_port = UDP_PORT; 
    strncpy(g_config.log_path, "./log", sizeof(g_config.log_path)); 
    
    if (pipe(g_sig_pipe) < 0) {
        perror("Failed to create signal pipe");
        exit(1);
    }
    set_nonblocking(g_sig_pipe[0]);
    set_nonblocking(g_sig_pipe[1]);

    LOG_INFO("Initializing IoT Gateway...");

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGPIPE, SIG_IGN); 

    if (argc == 3 && strcmp(argv[2], "-d") == 0) {
        LOG_INFO("Starting in daemon mode...");
        daemonize();
        strncpy(g_config.log_path, "/tmp/iot_log", sizeof(g_config.log_path)); 
    }
    
    if (init_ipc_modules() < 0) {
        LOG_FATAL("Failed to initialize System V IPC modules!");
        exit(1);
    }

    // [新增] 初始化进程池：创建 2 个隔离的子进程
    if (init_process_pool(2) < 0) {
        LOG_FATAL("Failed to initialize Process Pool!");
        destroy_ipc_modules();
        exit(1);
    }

    ThreadPool *pool = thread_pool_create(4, 100);
    if (pool == NULL) {
        LOG_FATAL("Failed to create thread pool!");
        destroy_process_pool();
        destroy_ipc_modules();
        exit(1);
    }

    LOG_INFO("Entering network event loop...");
    start_event_loop(pool, g_sig_pipe[0]);

    LOG_INFO("Gateway shutting down successfully. Cleaning up resources...");
    thread_pool_destroy(pool);
    destroy_process_pool(); // [新增] 释放进程池
    destroy_ipc_modules();
    close(g_sig_pipe[0]);
    close(g_sig_pipe[1]);
    
    return 0;
}