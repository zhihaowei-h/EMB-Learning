// 1.3.8 日志与监控需求
// src/log.c
#include "iot_gateway.h"
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

static FILE *log_fp = NULL;
static int current_log_day = -1;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

const char* level_strings[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

// 检查并自动切换/创建日志文件
static void check_and_rotate_log() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    if (t->tm_mday != current_log_day) {
        if (log_fp) {
            fclose(log_fp);
        }
        
        // 确保日志目录存在
        mkdir(g_config.log_path, 0755);
        
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/gateway_%04d%02d%02d.log", 
                 g_config.log_path, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
                 
        log_fp = fopen(filepath, "a");
        if (log_fp) {
            current_log_day = t->tm_mday;
        } else {
            fprintf(stderr, "Failed to open log file: %s\n", filepath);
        }
    }
}

void gateway_log(LogLevel level, const char *file, int line, const char *format, ...) {
    if (level < g_config.log_level) return;

    pthread_mutex_lock(&log_mutex);
    
    check_and_rotate_log();
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    // ... 前面的获取时间和组装逻辑保持不变 ...
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", t);
    
    // 提前把固定的日志头组装好
    char log_header[256];
    snprintf(log_header, sizeof(log_header), "[%s] [%s] %s:%d | ", time_buf, level_strings[level], file, line);
    
    va_list args;
    
    // 1. 如果文件指针正常，写入文件
    if (log_fp) {
        fprintf(log_fp, "%s", log_header);
        va_start(args, format);
        vfprintf(log_fp, format, args);
        va_end(args);
        fprintf(log_fp, "\n");
        fflush(log_fp);
    }
    
    // 2. 永远往标准输出打一份 (开发时看终端，守护进程时进黑洞)
    fprintf(stdout, "%s", log_header);
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    pthread_mutex_unlock(&log_mutex);
}