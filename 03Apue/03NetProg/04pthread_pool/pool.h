#pragma once

#include <pthread.h>
#include "queue.h"

#define MAXJOB 10     // 任务队列中的最大任务数
#define MIN_FREE_NR 3 // 线程池中最小空闲线程数
#define MAX_THREAD_NR 5 // 线程池中最大空闲线程数

// [1] 定义线程池结构体
typedef struct {
    pthread_t *workers;        // 工作线程结构的起始地址(动态开辟)
    pthread_t admin_tid;       // 管理线程ID
    queue_t *task_queue;        // 任务队列结构的起始地址(动态开辟)

    queue_t job_queue;         // 任务队列
    int max_threads;       // 线程池中最大线程数
    int shutdown;              // 关闭状态
    int exit_threads;         // 需要退出的线程数
    int live_threads;         // 线程池中存活的线程数
    int busy_threads;         // 线程池中忙碌的线程数
    pthread_mutex_t mut_pool;     // 整个线程池的锁
    pthread_mutex_t mut_busy;     // 忙线程的锁
    pthread_cond_t queue_not_empty; // 任务队列不为空的条件变量(如果任务队列为空,通知取任务)
    pthread_cond_t queue_not_full;  // 任务队列不为满的条件变量(如果任务队列满了,通知放任务)
} thread_pool_t;

// 任务结构体
typedef struct {
    void *(*job)(void *s); // 任务函数(存储执行任务的地址)
    void *arg;             // 任务函数的参数
}task_t;

// [2] 定义线程池相关函数


// 功能 : 线程池初始化
extern int pool_init(thread_pool_t **mypool, int capacity);

// 功能 : 线程池销毁
extern void pool_destroy(thread_pool_t *mypool);

// 功能 : 向线程池中添加任务
extern int pool_add_task(thread_pool_t *mypool, const task_t *t);