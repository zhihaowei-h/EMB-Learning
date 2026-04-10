#ifndef LOGHAWK_H
#define LOGHAWK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

// IPC Keys
#define SHM_KEY 0x1111
#define SEM_KEY 0x2222
#define MSG_KEY 0x3333

/* 5.1 共享内存环形缓冲区 [cite: 95] */
#define SHM_SIZE (1024 * 1024 * 4)  // 4MB [cite: 96]
struct shm_buffer {
    int write_pos;    // 写指针 [cite: 96]
    int read_pos;     // 读指针 [cite: 96]
    char data[SHM_SIZE]; // [cite: 96]
};

/* 5.2 消息队列结构体 [cite: 97] */
#define MSG_MAX 1024 // [cite: 98]
struct msgbuf {
    long mtype; // [cite: 98]
    char mtext[MSG_MAX]; // [cite: 98]
};

/* 5.3 令牌桶结构体 [cite: 99] */
struct token_bucket {
    int token;      // 当前令牌数 [cite: 100]
    int cps;        // 每秒生成令牌 [cite: 100]
    int burst;      // 最大容量 [cite: 100]
};

/* 5.4 采集偏移量记录 [cite: 101] */
struct log_offset {
    char filename[256]; // [cite: 102]
    long offset; // [cite: 102]
    ino_t inode; // [cite: 102]
};

// 信号量操作辅助函数
static inline void sem_p(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

static inline void sem_v(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

#endif // LOGHAWK_H