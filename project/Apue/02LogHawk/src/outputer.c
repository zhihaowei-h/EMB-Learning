/*
 * outputer.c —— 日志输出模块（守护进程）
 *
 * 职责：
 *   1. 以守护进程方式运行（后台，脱离终端）
 *   2. 从消息队列消费处理后的日志
 *   3. 令牌桶限流（防止写文件速度过快）
 *   4. 按级别写到 info.log / warn.log / error.log
 *
 * 与其他模块的关系：
 *   [消息队列] → Outputer → 分级日志文件
 */

#include "../inc/loghawk.h"

/* ============================================================
 * 全局变量
 * ============================================================ */
static int msgid = -1;
static volatile int running = 1;
static struct token_bucket tb;

/* ============================================================
 * 信号处理
 * ============================================================ */
static void handle_stop(int sig) {
    (void)sig;
    running = 0;
}

/* ============================================================
 * 主函数：输出循环
 * ============================================================ */
int outputer_run(int as_daemon) {
    /* 是否以守护进程方式运行 */
    if (as_daemon) {
        daemonize();
        /* daemonize() 之后，父进程已经退出，这里是子进程继续 */
    }

    log_info("[Outputer] 启动（PID=%d，守护进程=%s）",
             getpid(), as_daemon ? "是" : "否");

    signal(SIGTERM, handle_stop);
    signal(SIGINT,  handle_stop);

    /* 连接消息队列 */
    msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) {
        log_info("[Outputer] msgget 失败：%s", strerror(errno));
        return 1;
    }

    /* 初始化令牌桶：每秒最多输出 50 条，桶容量 100 */
    tb_init(&tb, 50, 100);

    struct log_msg msg;
    long dropped = 0;
    long written = 0;

    log_info("[Outputer] 开始消费消息队列，限流：50条/秒");

    while (running) {
        /*
         * 非阻塞接收：type=0 接收任意类型，IPC_NOWAIT 不阻塞
         * 没有消息时返回 -1，errno=ENOMSG
         */
        ssize_t n = msgrcv(msgid, &msg, MSG_TEXT_MAX, 0, IPC_NOWAIT);

        if (n > 0) {
            /* 有消息，检查令牌 */
            if (tb_consume(&tb)) {
                /* 令牌充足，按级别写文件 */
                switch (msg.mtype) {
                    case LEVEL_ERROR:
                        log_write(OUT_ERROR_LOG, msg.mtext);
                        break;
                    case LEVEL_WARN:
                        log_write(OUT_WARN_LOG, msg.mtext);
                        break;
                    case LEVEL_INFO:
                    default:
                        log_write(OUT_INFO_LOG, msg.mtext);
                        break;
                }
                written++;

                /* 每写 100 条打一条运行日志 */
                if (written % 100 == 0) {
                    log_info("[Outputer] 已写出 %ld 条，丢弃 %ld 条（限流）",
                             written, dropped);
                }
            } else {
                /* 令牌不足，限流丢弃 */
                dropped++;
                if (dropped % 10 == 0)
                    log_info("[Outputer] 限流：已丢弃 %ld 条", dropped);
            }
        } else {
            /* 队列为空，等一会儿 */
            usleep(20000);   /* 20ms */
        }
    }

    log_info("[Outputer] 退出，共写出 %ld 条，丢弃 %ld 条", written, dropped);

    /* 清理 PID 文件 */
    unlink(PID_FILE);
    return 0;
}
