/*
 * logger.c —— 日志写出工具
 * 负责把处理好的日志写到 output_logs/ 目录下的文件
 */

#include "../inc/loghawk.h"
#include <stdarg.h>

/* ============================================================
 * 写一条日志到指定文件（追加模式）
 * 会在内容前面加上时间戳
 * ============================================================ */
void log_write(const char *filepath, const char *content) {
    FILE *fp = fopen(filepath, "a");
    if (!fp) {
        /* 写不进去就算了，不影响主流程 */
        return;
    }

    /* 生成时间戳 */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(fp, "[%s] %s\n", timebuf, content);
    fclose(fp);
}

/* ============================================================
 * 写 LogHawk 自身的运行日志（用于调试）
 * 输出到 output_logs/loghawk.log
 * ============================================================ */
void log_info(const char *fmt, ...) {
    FILE *fp = fopen("output_logs/loghawk.log", "a");
    if (!fp) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(fp, "[%s] ", timebuf);

    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fprintf(fp, "\n");
    fclose(fp);
}
