/*===============================================
 * 文件名称：thread_pool.c
 * 创 建 者：IoT Gateway System
 * 创建日期：2026年04月15日
 * 描    述：线程池模块
 * 实现固定大小线程池、任务队列管理
 ================================================*/

#include "../inc/iot_gateway.h"

// 全局线程池
thread_pool_t *g_thread_pool = NULL;

/*
 * 功能：工作线程函数
 * 参数：arg - 线程池指针
 * 返回：NULL
 */
static void *worker_thread(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;
    task_t *task;
    
    // 【修改点】：屏蔽 SIGALRM 信号，防止被令牌桶定时器打断网络请求
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    
    while (1) {
        pthread_mutex_lock(&pool->lock);
        
        // 等待任务或关闭信号
        while (pool->task_queue_head == NULL && !pool->shutdown) {
            pthread_cond_wait(&pool->notify, &pool->lock);
        }
        
        // 如果收到关闭信号且任务队列为空，退出线程
        if (pool->shutdown && pool->task_queue_head == NULL) {
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        
        // 从队列头取出一个任务
        task = pool->task_queue_head;
        if (task != NULL) {
            pool->task_queue_head = task->next;
            if (pool->task_queue_head == NULL) {
                pool->task_queue_tail = NULL;
            }
            pool->task_count--;
        }
        
        pthread_mutex_unlock(&pool->lock);
        
        // 执行任务
        if (task != NULL) {
            (*(task->function))(task->arg);
            free(task);
        }
    }
    
    pthread_exit(NULL);
    return NULL;
}

/*
 * 功能：创建线程池
 * 参数：thread_count - 线程数量
 * 返回：线程池指针，失败返回NULL
 */
thread_pool_t *thread_pool_create(int thread_count)
{
    thread_pool_t *pool;
    int i;
    
    if (thread_count <= 0) {
        fprintf(stderr, "[ERROR] Invalid thread count\n");
        return NULL;
    }
    
    // 分配线程池结构
    pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (pool == NULL) {
        perror("malloc");
        return NULL;
    }
    
    // 初始化线程池
    pool->thread_count = thread_count;
    pool->task_queue_head = NULL;
    pool->task_queue_tail = NULL;
    pool->task_count = 0;
    pool->shutdown = 0;
    
    // 初始化互斥锁和条件变量
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->notify, NULL);
    
    // 分配线程数组
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    if (pool->threads == NULL) {
        perror("malloc");
        free(pool);
        return NULL;
    }
    
    // 创建工作线程
    for (i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            perror("pthread_create");
            thread_pool_destroy(pool);
            return NULL;
        }
    }
    
    log_write(LOG_INFO, "Thread pool created with %d threads", thread_count);
    
    return pool;
}

/*
 * 功能：向线程池添加任务
 * 参数：pool - 线程池指针
 * function - 任务函数
 * arg - 任务参数
 * 返回：成功返回0，失败返回-1
 */
int thread_pool_add_task(thread_pool_t *pool, void (*function)(void *), void *arg)
{
    task_t *task;
    
    if (pool == NULL || function == NULL) {
        return -1;
    }
    
    // 分配任务结构
    task = (task_t *)malloc(sizeof(task_t));
    if (task == NULL) {
        perror("malloc");
        return -1;
    }
    
    // 初始化任务
    task->function = function;
    task->arg = arg;
    task->next = NULL;
    
    pthread_mutex_lock(&pool->lock);
    
    // 检查是否已经关闭
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->lock);
        free(task);
        return -1;
    }
    
    // 将任务加入队列尾部
    if (pool->task_queue_tail == NULL) {
        pool->task_queue_head = task;
        pool->task_queue_tail = task;
    } else {
        pool->task_queue_tail->next = task;
        pool->task_queue_tail = task;
    }
    
    pool->task_count++;
    
    // 通知一个工作线程
    pthread_cond_signal(&pool->notify);
    
    pthread_mutex_unlock(&pool->lock);
    
    return 0;
}

/*
 * 功能：销毁线程池
 * 参数：pool - 线程池指针
 * 返回：无
 */
void thread_pool_destroy(thread_pool_t *pool)
{
    int i;
    task_t *task, *next;
    
    if (pool == NULL) {
        return;
    }
    
    pthread_mutex_lock(&pool->lock);
    
    // 设置关闭标志
    pool->shutdown = 1;
    
    // 唤醒所有线程
    pthread_cond_broadcast(&pool->notify);
    
    pthread_mutex_unlock(&pool->lock);
    
    // 等待所有线程退出
    for (i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // 释放剩余任务
    task = pool->task_queue_head;
    while (task != NULL) {
        next = task->next;
        free(task);
        task = next;
    }
    
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
    
    // 释放内存
    free(pool->threads);
    free(pool);
    
    log_write(LOG_INFO, "Thread pool destroyed");
}