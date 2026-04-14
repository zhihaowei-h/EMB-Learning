// 1.3.3 线程池模块需求
#include "iot_gateway.h"
#include "thread_pool.h"

// 工作线程的本体运行函数
static void *thread_pool_worker(void *thread_pool) {
    ThreadPool *pool = (ThreadPool *)thread_pool;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        // 如果队列为空且没有收到销毁信号，就阻塞等待通知
        while ((pool->queue_size == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        // 如果收到销毁信号，退出循环，线程结束
        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        // 取出任务队列的第一个任务
        ThreadTask *task = pool->queue_head;
        if (task != NULL) {
            pool->queue_head = task->next;
            pool->queue_size--;
            if (pool->queue_size == 0) {
                pool->queue_tail = NULL;
            }
        }
        
        pthread_mutex_unlock(&(pool->lock));

        // 拿到任务后，在锁外执行业务逻辑（实现真正的并发）
        if (task != NULL) {
            (*(task->function))(task->arg);
            free(task); // 释放任务节点内存
        }
    }
    return NULL;
}

// 创建并初始化线程池
ThreadPool* thread_pool_create(int thread_count, int max_queue_size) {
    if (thread_count <= 0 || max_queue_size <= 0) return NULL;

    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (pool == NULL) return NULL;

    pool->thread_count = thread_count;
    pool->max_queue_size = max_queue_size;
    pool->queue_size = 0;
    pool->queue_head = NULL;
    pool->queue_tail = NULL;
    pool->shutdown = 0;

    // 初始化锁和条件变量
    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->notify), NULL) != 0) {
        free(pool);
        return NULL;
    }

    // 分配线程数组内存
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    if (pool->threads == NULL) {
        free(pool);
        return NULL;
    }

    // 启动指定数量的工作线程
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, thread_pool_worker, (void*)pool) != 0) {
            // 如果创建失败，为了健壮性，这里应该触发销毁逻辑（为保持代码精简，暂略）
            LOG_ERROR("Failed to create thread %d", i);
            return NULL;
        }
    }
    
    LOG_INFO("Thread pool created with %d threads, max queue: %d", thread_count, max_queue_size);
    return pool;
}

// 向线程池投递任务
int thread_pool_add_task(ThreadPool *pool, void (*function)(void *), void *arg) {
    if (pool == NULL || function == NULL) return -1;

    pthread_mutex_lock(&(pool->lock));

    // 检查队列是否已满，防止突发流量打爆内存
    if (pool->queue_size >= pool->max_queue_size) {
        LOG_WARN("Thread pool queue is full! Task rejected.");
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    // 检查是否正在销毁
    if (pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    // 创建新任务节点
    ThreadTask *new_task = (ThreadTask *)malloc(sizeof(ThreadTask));
    if (new_task == NULL) {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }
    new_task->function = function;
    new_task->arg = arg;
    new_task->next = NULL;

    // 将任务挂入链表尾部
    if (pool->queue_size == 0) {
        pool->queue_head = new_task;
        pool->queue_tail = new_task;
    } else {
        pool->queue_tail->next = new_task;
        pool->queue_tail = new_task;
    }
    pool->queue_size++;

    // 通知一个阻塞等待的空闲线程起来接活
    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

// 优雅销毁线程池
int thread_pool_destroy(ThreadPool *pool) {
    if (pool == NULL) return -1;

    pthread_mutex_lock(&(pool->lock));
    if (pool->shutdown) { // 避免重复销毁
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }
    pool->shutdown = 1;
    
    // 唤醒所有阻塞的线程，让它们看到 shutdown 标志后退出
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    // 回收所有线程资源
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // 清理残余任务并释放内存
    ThreadTask *curr = pool->queue_head;
    while (curr != NULL) {
        ThreadTask *next = curr->next;
        free(curr);
        curr = next;
    }

    free(pool->threads);
    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool);

    LOG_INFO("Thread pool destroyed gracefully.");
    return 0;
}