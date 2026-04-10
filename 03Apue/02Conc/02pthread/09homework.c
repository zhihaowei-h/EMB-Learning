#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define PTHREAD_NUM 5  // 定义线程数量
/**
 * job = -1: 此时没有任务
 * job = 0: 表示要打印'a'
 * job = 1: 表示要打印'b'
 * job = 2: 表示要打印'c'
 * job = 3: 表示要打印'd'
 * job = 4: 表示要打印'e'
 */
static int job = -1; // 定义一个全局变量job，初始值为-1，表示没有任务发放

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 定义一个互斥锁，并初始化
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // 定义一个条件变量，并初始化

// 线程函数，负责执行任务
static void *thread_job(void *arg){
    int thread_id = *(int *)arg; // 获取线程编号

    while(1){
        pthread_mutex_lock(&mutex); // 加锁，保护共享资源(或叫临界资源)job
        while(job != thread_id){
            pthread_cond_wait(&cond, &mutex); // 如果当前线程的编号与job不相同，解锁并继续等待
        }
        putchar(thread_id + 'a'); // 根据编号打印对应的字符
        fflush(NULL); // 刷新输出缓冲区，确保字符立即显示 // FIXME
        job = -1; // 重置job为-1，表示任务已经完成
        pthread_cond_broadcast(&cond); // 发送条件变量变化的通知
        pthread_mutex_unlock(&mutex); // 解锁，允许其他线程访问共享资源
    }
}



int main(){
    pthread_t tids[PTHREAD_NUM]; // 定义一个数组来存储线程ID
    int thread_ids[PTHREAD_NUM]; // 定义一个数组来存储线程编号

    alarm(5); // 设置一个闹钟，5秒后发送SIGALRM信号，终止程序

    // 创建多个线程，每个线程负责打印一个特定的字符
    for(int i = 0; i < PTHREAD_NUM; i++){
        thread_ids[i] = i; // 初始化线程编号
        int ret = pthread_create(&tids[i], NULL, thread_job, (void *)&thread_ids[i]); // 创建线程
        // 如果 线程创建失败
        if (ret != 0) {
            fprintf(stderr, "pthread_create(): %s\n", strerror(ret)); // 输出错误信息
            exit(1); // 由于创建线程失败，退出进程，并返回错误码1
        }
    }

    // 主线程负责发放任务，按照顺序打印字符
    for(int i = 0; ; i = (i + 1) % PTHREAD_NUM){ // 无限循环，按照顺序发放任务(i = 0, 1, 2, 3, 4, 0, 1, ...)
        pthread_mutex_lock(&mutex); // 加锁，保护共享资源job
        // 如果 任务池中还有任务没有被取走
        while(job != -1){
            pthread_cond_wait(&cond, &mutex); // 如果job不为-1，说明任务还没有完成，解锁并继续等待
        }        
        job = i; // 发放任务: 设置job为当前线程编号
        pthread_cond_broadcast(&cond); // 发送条件变量变化的通知，唤醒等待的线程
        pthread_mutex_unlock(&mutex); // 解锁，允许线程访问共享资源
    }

    // 程序将在5秒后被SIGALRM信号终止，因此不需要显式地等待线程结束

    // 循环等待线程结束，收尸
    for(int i = 0; i < PTHREAD_NUM; i++){
        pthread_join(tids[i], NULL);
    }

    pthread_mutex_destroy(&mutex); // 销毁互斥锁
    pthread_cond_destroy(&cond); // 销毁条件变量

    return 0;
}