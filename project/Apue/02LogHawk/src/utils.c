/*
 * utils.c —— 工具函数
 * 包含：断点续传偏移量读写、日志级别解析、守护进程化
 */

#include "../inc/loghawk.h"
#include <stdarg.h>

/* ============================================================
 * 断点续传：偏移量持久化
 * 把上次读到 /var/log/syslog 哪个字节记到文件里
 * 程序重启后可以从上次的位置继续，不重复读旧日志
 * ============================================================ */

/* 从文件读取上次保存的偏移量，若文件不存在则返回 -1（表示从末尾开始） */
long offset_load(void) {
    FILE *fp = fopen(OFFSET_FILE, "r");
    if (!fp) return -1;   /* 第一次运行，文件不存在 */
    long offset = 0;
    fscanf(fp, "%ld", &offset);
    fclose(fp);
    return offset;
}

/* 把当前偏移量写到文件 */
void offset_save(long offset) {
    FILE *fp = fopen(OFFSET_FILE, "w");
    if (!fp) return;
    fprintf(fp, "%ld\n", offset);
    fclose(fp);
}

/* ============================================================
 * 日志级别解析
 * 从一行 syslog 文本里判断它是 INFO/WARN/ERROR
 *
 * Ubuntu syslog 格式示例：
 *   Apr  1 10:23:45 hostname kernel: [12345.678] something
 *   Apr  1 10:23:46 hostname systemd[1]: Started xxx.
 *
 * 我们通过关键字匹配来判断级别
 * ============================================================ */
int parse_level(const char *line) {

    /* syslog 协议优先级标记，格式是 <数字>，数字含义：
     * 0-2 = emergency/alert/critical → ERROR
     * 3   = error                    → ERROR
     * 4   = warning                  → WARN
     * 5-6 = notice/info              → INFO
     */
    if (strstr(line, "emerg")    ||
        strstr(line, "alert")    ||
        strstr(line, "crit")     ||
        strstr(line, "Critical")) return LEVEL_ERROR;

    /* 明确的 error 关键字 */
    if (strstr(line, "error")    ||
        strstr(line, "Error")    ||
        strstr(line, "ERROR")    ||
        strstr(line, "failed")   ||   /* ← 新增：systemd 常用 */
        strstr(line, "Failed")   ||
        strstr(line, "FAILED")   ||
        strstr(line, "fail")     ||
        strstr(line, "Fail")     ||
        strstr(line, "panic")    ||
        strstr(line, "segfault") ||   /* ← 新增：内核段错误 */
        strstr(line, "OOM")      ||   /* ← 新增：内存不足 */
        strstr(line, "killed"))       /* ← 新增：进程被杀 */
        return LEVEL_ERROR;

    /* warn 关键字 */
    if (strstr(line, "warn")     ||
        strstr(line, "Warn")     ||
        strstr(line, "WARNING")  ||
        strstr(line, "deprecated") ||  /* ← 新增 */
        strstr(line, "timeout")  ||    /* ← 新增：超时算警告 */
        strstr(line, "Timeout")  ||
        strstr(line, "retrying") ||    /* ← 新增：重试算警告 */
        strstr(line, "slow"))          /* ← 新增：慢响应算警告 */
        return LEVEL_WARN;

    /* info 关键字 */
    if (strstr(line, "info")     ||
        strstr(line, "Info")     ||
        strstr(line, "INFO")     ||
        strstr(line, "started")  ||
        strstr(line, "Started")  ||
        strstr(line, "stopped")  ||    /* ← 新增 */
        strstr(line, "Stopped")  ||
        strstr(line, "loaded")   ||    /* ← 新增：systemd 常用 */
        strstr(line, "Loaded")   ||
        strstr(line, "activated") ||
        strstr(line, "running"))       /* ← 新增 */
        return LEVEL_INFO;

    return LEVEL_INFO;
}

/* ============================================================
 * 守护进程化
 * 让 Outputer 脱离终端，在后台 7x24 运行
 * ============================================================ */
void daemonize(void) {
    /* 第一次 fork：父进程退出，shell 提示符立即返回 */
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid > 0) exit(0);   /* 父进程退出 */

    /* 子进程创建新会话，彻底脱离原来的终端控制组 */
    if (setsid() < 0) { perror("setsid"); exit(1); }

    /* 第二次 fork：防止子进程重新获得控制终端 */
    pid = fork();
    if (pid < 0) { perror("fork2"); exit(1); }
    if (pid > 0) exit(0);

    /* 设置工作目录，避免占用可卸载的挂载点 */
    chdir("/tmp");

    /* 设置 umask，确保创建的文件权限可控 */
    umask(0);

    /* 关闭标准输入输出（守护进程不需要终端） */
    int devnull = open("/dev/null", O_RDWR);
    dup2(devnull, STDIN_FILENO);
    dup2(devnull, STDOUT_FILENO);
    dup2(devnull, STDERR_FILENO);
    close(devnull);

    /* 写 PID 文件，方便外部 kill */
    FILE *fp = fopen(PID_FILE, "w");
    if (fp) {
        fprintf(fp, "%d\n", getpid());
        fclose(fp);
    }
}
