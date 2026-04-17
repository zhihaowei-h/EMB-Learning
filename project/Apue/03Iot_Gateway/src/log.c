/*===============================================
 *   文件名称：log.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：日志模块实现
 *            支持分级日志、按天滚动、线程安全
 ================================================*/

#include "../inc/iot_gateway.h"
#include <stdarg.h>
#include <sys/stat.h>

// 日志级别字符串
static const char *log_level_str[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

// 日志文件指针
static FILE *g_log_fp = NULL;
static char g_log_dir[LOG_PATH_SIZE] = {0};
static log_level_t g_log_level = LOG_INFO;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static char g_current_date[16] = {0};

/*
 * 功能：初始化日志系统
 * 参数：log_path - 日志目录路径
 *       level - 日志级别
 * 返回：无
 */
void log_init(const char *log_path, log_level_t level)
{
    struct stat st = {0};
    
    // 保存日志目录
    strncpy(g_log_dir, log_path, sizeof(g_log_dir) - 1);
    g_log_level = level;
    
    // 创建日志目录（如果不存在）
    if (stat(g_log_dir, &st) == -1) {
        mkdir(g_log_dir, 0755);
    }
    
    printf("[LOG] Log system initialized: path=%s, level=%s\n", 
           g_log_dir, log_level_str[g_log_level]);
}

/*
 * 功能：获取当前日期字符串
 * 参数：无
 * 返回：日期字符串指针
 */
static const char *get_current_date(void)
{
    static char date_buf[16];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    
    return date_buf;
}

/*
 * 功能：打开当天的日志文件
 * 参数：无
 * 返回：成功返回0，失败返回-1
 */
static int open_log_file(void)
{
    char log_file[LOG_PATH_SIZE + 32];
    const char *date = get_current_date();
    
    // 检查日期是否变化，需要滚动日志
    if (strcmp(g_current_date, date) != 0) {
        // 关闭旧的日志文件
        if (g_log_fp != NULL) {
            fclose(g_log_fp);
            g_log_fp = NULL;
        }
        
        // 生成新的日志文件路径
        snprintf(log_file, sizeof(log_file), "%s/gateway_%s.log", 
                 g_log_dir, date);
        
        // 打开新的日志文件（追加模式）
        g_log_fp = fopen(log_file, "a");
        if (g_log_fp == NULL) {
            fprintf(stderr, "[ERROR] Failed to open log file: %s\n", log_file);
            return -1;
        }
        
        // 设置为行缓冲
        setlinebuf(g_log_fp);
        
        // 更新当前日期
        strncpy(g_current_date, date, sizeof(g_current_date) - 1);
        
        fprintf(g_log_fp, "\n========== Log file created: %s ==========\n\n", date);
    }
    
    return 0;
}

/*
 * 功能：写入日志
 * 参数：level - 日志级别
 *       fmt - 格式化字符串
 *       ... - 可变参数
 * 返回：无
 */
void log_write(log_level_t level, const char *fmt, ...)
{
    char log_buf[BUFFER_SIZE];
    va_list args;
    time_t now;
    struct tm *t;
    
    // 检查日志级别
    if (level < g_log_level) {
        return;
    }
    
    // 加锁保证线程安全
    pthread_mutex_lock(&g_log_mutex);
    
    // 打开日志文件（检查是否需要滚动）
    if (open_log_file() < 0) {
        pthread_mutex_unlock(&g_log_mutex);
        return;
    }
    
    // 获取当前时间
    now = time(NULL);
    t = localtime(&now);
    
    // 格式化日志消息
    va_start(args, fmt);
    vsnprintf(log_buf, sizeof(log_buf), fmt, args);
    va_end(args);
    
    // 写入日志文件
    fprintf(g_log_fp, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            log_level_str[level], log_buf);
    
    // 如果是ERROR或FATAL级别，同时输出到stderr
    if (level >= LOG_ERROR) {
        fprintf(stderr, "[%s] %s\n", log_level_str[level], log_buf);
    }
    
    pthread_mutex_unlock(&g_log_mutex);
}

/*
 * 功能：关闭日志系统
 * 参数：无
 * 返回：无
 */
void log_close(void)
{
    pthread_mutex_lock(&g_log_mutex);
    
    if (g_log_fp != NULL) {
        fprintf(g_log_fp, "\n========== Log system closed ==========\n");
        fclose(g_log_fp);
        g_log_fp = NULL;
    }
    
    pthread_mutex_unlock(&g_log_mutex);
    
    printf("[LOG] Log system closed\n");
}
