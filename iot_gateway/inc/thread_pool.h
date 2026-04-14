#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

// 任务节点结构（链表）
typedef struct ThreadTask {
    void (*function)(void *); // 任务执行的函数指针
    void *arg;                // 函数参数
    struct ThreadTask *next;  // 指向下一个任务
} ThreadTask;

// 线程池结构体
typedef struct {
    pthread_mutex_t lock;       // 保护任务队列的互斥锁
    pthread_cond_t notify;      // 通知工作线程的条件变量
    pthread_t *threads;         // 线程数组
    ThreadTask *queue_head;     // 任务队列头
    ThreadTask *queue_tail;     // 任务队列尾
    int thread_count;           // 线程数量
    int queue_size;             // 当前队列中的任务数
    int max_queue_size;         // 队列最大允许长度（防过载 OOM）
    int shutdown;               // 线程池销毁标志
} ThreadPool;

// --- 接口声明 ---
ThreadPool* thread_pool_create(int thread_count, int max_queue_size);
int thread_pool_add_task(ThreadPool *pool, void (*function)(void *), void *arg);
int thread_pool_destroy(ThreadPool *pool);

#endif // THREAD_POOL_H