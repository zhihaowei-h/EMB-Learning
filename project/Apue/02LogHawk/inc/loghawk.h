#ifndef LOGHAWK_H
#define LOGHAWK_H

/* ============================================================
 * loghawk.h —— 全局公共定义
 * 所有模块都 #include 这个头文件
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <errno.h>

/* ============================================================
 * 路径配置
 * ============================================================ */
#define SOURCE_LOG      "/var/log/syslog"       /* Ubuntu 系统日志源 */
#define OUT_INFO_LOG    "output_logs/info.log"  /* INFO 级别输出 */
#define OUT_WARN_LOG    "output_logs/warn.log"  /* WARN 级别输出 */
#define OUT_ERROR_LOG   "output_logs/error.log" /* ERROR 级别输出 */
#define OFFSET_FILE     "/tmp/loghawk.offset"   /* 断点续传偏移量记录 */
#define PID_FILE        "/tmp/loghawk.pid"      /* 守护进程 PID 文件 */

/* ============================================================
 * IPC 键值（进程间通信的"门牌号"，每个必须唯一）
 * ============================================================ */
#define SHM_KEY     0x4C4F4731   /* 共享内存 key（"LOG1"） */
#define SEM_KEY     0x4C4F4732   /* 信号量 key */
#define MSG_KEY     0x4C4F4733   /* 消息队列 key */

/* ============================================================
 * 共享内存：环形缓冲区
 * Collector 写入，Processor 读取
 * ============================================================ */
#define SHM_SLOTS   64           /* 环形缓冲槽数量 */
#define LINE_MAX    512          /* 每条日志最大长度 */

/* 一个缓冲槽 */
struct shm_slot {
    int  used;               /* 0=空闲，1=有数据待处理 */
    char data[LINE_MAX];     /* 日志原文 */
};

/* 共享内存整体布局 */
struct shm_ring {
    int write_idx;                   /* Collector 下次写到哪个槽 */
    int read_idx;                    /* Processor 下次从哪个槽读 */
    struct shm_slot slots[SHM_SLOTS];
};

/* ============================================================
 * 消息队列：Processor → Outputer
 * ============================================================ */
#define MSG_TEXT_MAX  512

/* 日志级别（同时用作消息类型 mtype） */
#define LEVEL_INFO    1
#define LEVEL_WARN    2
#define LEVEL_ERROR   3
#define LEVEL_OTHER   4   /* 其他/无法识别 */

struct log_msg {
    long mtype;                /* 消息类型（即日志级别） */
    char mtext[MSG_TEXT_MAX];  /* 格式化后的日志内容 */
};

/* ============================================================
 * 进程池配置
 * ============================================================ */
#define WORKER_COUNT  3   /* Processor 工人数量，建议设为 CPU 核数 */

/* ============================================================
 * 令牌桶（流量控制）
 * ============================================================ */
struct token_bucket {
    int     tokens;       /* 当前令牌数 */
    int     cps;          /* 每秒生成令牌数（条/秒） */
    int     burst;        /* 桶最大容量 */
    time_t  last_refill;  /* 上次补充时间 */
};

/* ============================================================
 * 函数声明（各模块对外暴露的接口）
 * ============================================================ */

/* ipc.c */
int  ipc_shm_create(void);
struct shm_ring *ipc_shm_attach(int shmid);
void ipc_shm_detach(struct shm_ring *ring);
void ipc_shm_destroy(int shmid);
int  ipc_sem_create(void);
void ipc_sem_lock(int semid);
void ipc_sem_unlock(int semid);
void ipc_sem_destroy(int semid);
int  ipc_msg_create(void);
void ipc_msg_destroy(int msgid);

/* token_bucket.c */
void tb_init(struct token_bucket *tb, int cps, int burst);
int  tb_consume(struct token_bucket *tb);

/* logger.c */
void log_write(const char *filepath, const char *content);
void log_info(const char *fmt, ...);  /* 写到 loghawk 自己的运行日志 */

/* utils.c */
long  offset_load(void);
void  offset_save(long offset);
int   parse_level(const char *line);
void  daemonize(void);

#endif /* LOGHAWK_H */
