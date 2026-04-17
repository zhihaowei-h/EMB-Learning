/*===============================================
 *   文件名称：main.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：主程序入口
 *            整合所有模块，实现完整的网关功能
 ================================================*/

#include "../inc/iot_gateway.h"

/*
 * 功能：打印使用帮助
 * 参数：prog_name - 程序名
 * 返回：无
 */
static void print_usage(const char *prog_name)
{
    printf("Usage: %s <config_file> [-d]\n", prog_name);
    printf("  -d: Run as daemon\n");
    printf("\nExample:\n");
    printf("  %s ./iot_gateway.conf\n", prog_name);
    printf("  %s ./iot_gateway.conf -d\n", prog_name);
}

/*
 * 功能：定时任务 - 令牌桶补充
 * 参数：无
 * 返回：无
 */
static void task_token_refill(void)
{
    token_bucket_refill();
}

/*
 * 功能：定时任务 - 心跳保活
 * 参数：无
 * 返回：无
 */
static void task_heartbeat(void)
{
    log_write(LOG_DEBUG, "Heartbeat: running=%d, clients=%d", 
              g_running, g_client_count);
}

/*
 * 功能：定时任务 - 状态上报
 * 参数：无
 * 返回：无
 */
static void task_status_report(void)
{
    log_write(LOG_INFO, "Status: clients=%d, sensors=%d, uptime=%ld",
              g_client_count, 
              g_shm_ptr ? g_shm_ptr->sensor_count : 0,
              time(NULL) - g_shm_ptr->last_update);
}

/*
 * 功能：主函数
 * 参数：argc - 参数个数
 *       argv - 参数数组
 * 返回：成功返回0，失败返回1
 */
int main(int argc, char *argv[])
{
    int daemon_mode = 0;
    
    // 检查参数
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 检查是否守护进程模式
    if (argc >= 3 && strcmp(argv[2], "-d") == 0) {
        daemon_mode = 1;
    }
    
    printf("\n");
    printf("========================================\n");
    printf("   IoT Gateway System v1.0\n");
    printf("========================================\n");
    printf("\n");
    
    // 1. 加载配置文件
    printf("[INIT] Loading configuration...\n");
    if (config_load(argv[1]) < 0) {
        fprintf(stderr, "[ERROR] Failed to load configuration\n");
        return 1;
    }
    config_print();
    
    // 2. 初始化日志系统
    printf("[INIT] Initializing log system...\n");
    log_init(g_config.log_path, g_config.log_level);
    log_write(LOG_INFO, "========== IoT Gateway Started ==========");
    log_write(LOG_INFO, "Version: 1.0");
    log_write(LOG_INFO, "Build: %s %s", __DATE__, __TIME__);
    
    // 3. 守护进程化（如果需要）
    if (daemon_mode) {
        printf("[INIT] Daemonizing...\n");
        if (daemonize() < 0) {
            log_write(LOG_FATAL, "Failed to daemonize");
            return 1;
        }
        log_write(LOG_INFO, "Running as daemon");
    }
    
    // 4. 创建PID文件
    log_write(LOG_INFO, "Creating PID file...");
    if (create_pid_file() < 0) {
        log_write(LOG_FATAL, "Failed to create PID file");
        return 1;
    }
    
    // 5. 设置信号处理
    log_write(LOG_INFO, "Setting up signal handlers...");
    setup_signal_handlers();
    
    // 6. 初始化IPC资源
    log_write(LOG_INFO, "Initializing IPC resources...");
    if (ipc_init() < 0) {
        log_write(LOG_FATAL, "Failed to initialize IPC");
        goto cleanup;
    }
    
    // 7. 创建线程池
    log_write(LOG_INFO, "Creating thread pool...");
    g_thread_pool = thread_pool_create(g_config.thread_pool_size);
    if (g_thread_pool == NULL) {
        log_write(LOG_FATAL, "Failed to create thread pool");
        goto cleanup;
    }
    
    // 8. 创建进程池
    log_write(LOG_INFO, "Creating process pool...");
    g_process_pool = process_pool_create(g_config.process_pool_size);
    if (g_process_pool == NULL) {
        log_write(LOG_FATAL, "Failed to create process pool");
        goto cleanup;
    }
    
    // 9. 创建令牌桶
    log_write(LOG_INFO, "Creating token bucket...");
    g_token_bucket = token_bucket_create(g_config.token_cps, g_config.token_burst);
    if (g_token_bucket == NULL) {
        log_write(LOG_FATAL, "Failed to create token bucket");
        goto cleanup;
    }
    
    // 10. 初始化闹钟调度器
    log_write(LOG_INFO, "Initializing alarm scheduler...");
    alarm_scheduler_init();
    
    // 注册定时任务
    alarm_scheduler_add(1, task_token_refill);    // 每秒补充令牌
    alarm_scheduler_add(30, task_heartbeat);      // 每30秒心跳
    alarm_scheduler_add(60, task_status_report);  // 每60秒状态上报
    
    // 11. 初始化IO系统
    log_write(LOG_INFO, "Initializing IO system...");
    if (io_init() < 0) {
        log_write(LOG_FATAL, "Failed to initialize IO system");
        goto cleanup;
    }
    
    // 12. 进入主事件循环
    log_write(LOG_INFO, "========== System Ready ==========");
    log_write(LOG_INFO, "TCP Port: %d", g_config.tcp_port);
    log_write(LOG_INFO, "UDP Port: %d", g_config.udp_port);
    log_write(LOG_INFO, "Thread Pool: %d threads", g_config.thread_pool_size);
    log_write(LOG_INFO, "Process Pool: %d processes", g_config.process_pool_size);
    log_write(LOG_INFO, "==================================");
    
    io_loop();
    
cleanup:
    // 13. 清理资源
    log_write(LOG_INFO, "========== Shutting Down ==========");
    
    log_write(LOG_INFO, "Cleaning up IO resources...");
    io_cleanup();
    
    log_write(LOG_INFO, "Destroying alarm scheduler...");
    alarm_scheduler_destroy();
    
    log_write(LOG_INFO, "Destroying token bucket...");
    if (g_token_bucket != NULL) {
        token_bucket_destroy(g_token_bucket);
    }
    
    log_write(LOG_INFO, "Destroying process pool...");
    if (g_process_pool != NULL) {
        process_pool_destroy(g_process_pool);
    }
    
    log_write(LOG_INFO, "Destroying thread pool...");
    if (g_thread_pool != NULL) {
        thread_pool_destroy(g_thread_pool);
    }
    
    log_write(LOG_INFO, "Cleaning up IPC resources...");
    ipc_cleanup();
    
    log_write(LOG_INFO, "Removing PID file...");
    remove_pid_file();
    
    log_write(LOG_INFO, "========== Shutdown Complete ==========");
    log_close();
    
    return 0;
}
