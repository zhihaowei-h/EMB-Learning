#include <stdio.h>
#include "pool.h"
#include <stdlib.h>
#include <string.h>

int pool_init(thread_pool_t **mypool, int capacity) {
    thread_pool_t *me = NULL;
    me = malloc(sizeof(thread_pool_t)); // 开辟线程池结构体变量空间
    // 如果 开辟线程池空间失败
    if(me == NULL) {
        perror("malloc thread pool failed");
        return -1;
    }

    me->max_threads = capacity; // 线程池中最大线程数
    me->shutdown = 0; // 关闭状态
    me->exit_threads = 0; // 需要退出的线程数
    me->live_threads = 0; // 线程池中存活的线程数
    me->busy_threads = 0; // 线程池中忙碌的线程
    me->workers = calloc(capacity, sizeof(pthread_t)); // 开辟工作线程结构的空间
    // 如果 开辟工作线程结构的空间失败
    if(me->workers == NULL) {
        perror("malloc workers failed");
        free(me);
        return -1;
    }
    me->task_queue = malloc(sizeof(queue_t)); // 开辟任务队列结构的空间
    // 如果 开辟任务队列结构的空间失败
    if(me->task_queue == NULL) {
        perror("malloc task_queue failed");
        free(me->workers);
        free(me);
        return -1;
    }
    // 初始化任务队列
    me->task_queue->s = malloc(sizeof(task_t) * MAXJOB); // 开辟任务队列中存储任务的空间
    // 如果 开辟任务队列中存储任务的空间失败
    if(me->task_queue->s == NULL) {
        perror("malloc task_queue s failed");
        free(me->task_queue);
        free(me->workers);
        free(me);
        return -1;
    }
    // 互斥量和条件变量的初始化
    pthread_mutex_init(&me->mut_pool, NULL); // 初始化整个线程池的锁
    pthread_mutex_init(&me->mut_busy, NULL); // 初始化忙线程的锁
    pthread_cond_init(&me->queue_not_empty, NULL); // 初始化任务队列不为空的条件变量
    pthread_cond_init(&me->queue_not_full, NULL); // 初始化任务队列不为满的条件变量

    memset(me->workers, -1, sizeof(pthread_t) * capacity); // me->workers每一个成员初始化为-1(为了告诉使用者这个位置上没有线程)
   
    err = pthread_create(&me->admin_tid, NULL, admin_job, me); // 创建管理者线程
    // 如果 创建管理者线程失败
    if(err != 0){
        perror("pthread_create admin_job failed");
        free(me->workers); // 释放工作线程结构的空间
        free(me); // 释放线程池结构体变量空间
        return -err;
    }

    // 创建线程池中最少free线程数的线程
    int i = 0;
    for(i = 0; i < me->min_free_threads; i++) {
        err = pthread_create(me->workers + i, NULL, worker_job, me); // 创建工作    线程
        if(err != 0) {
            perror("pthread_create worker_job failed");
            free(me->workers); // 释放工作线程结构的空间
            free(me); // 释放线程池结构体变量空间
            return -err;
        }
        // 线程分离(线程结束后自动回收资源，将来不用pthread_join回收线程资源了)
        pthread_detach(me->workers[i]);
    }
    *mypool = me; // 线程池结构体变量的地址回填给mypool

    return 0;
}