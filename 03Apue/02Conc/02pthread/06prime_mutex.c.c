// FIXME    
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MIN 100
#define MAX 300
#define NUM_THREADS 4 // 定义要创建的线程的数量

/**
 * 当job = 0: 代表此时没有任务
 * 当job > 0: 代表此时有任务
 * 当job = -1: 代表所有任务发放完毕，且都被取走
 * 
 * 注意: 
 * 1. job变量是全局变量，由多个线程共享，所以它是临界资源，容易出现竞态，为了避免出现竞态，
 * 所以访问它必须加锁(即互斥量)，保证同一时刻只有一个线程访问它
 * 2. 线程在访问job变量之前，必须先加锁，访问完job变量之后，必须解锁
 * 3. 线程在访问job变量之前，必须先判断job变量的值，如果job变量的值为0，说明任务还没有发放，线程就等待；如果job变量的值大于0，说明任务已经发放
 */
static int job;  // 临界资源，代表任务的编号，初始值为0，表示没有任务

// [1]初始化互斥量(要么调用pthread_mutex_init()函数，要么使用PTHREAD_MUTEX_INITIALIZER宏来初始化互斥量)
// 这里我们使用PTHREAD_MUTEX_INITIALIZER宏来初始化互斥量，所以不需要调用pthread_mutex_init()函数来初始化互斥量了
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 定义一个互斥量，并初始化

// 判断num是否为质数，如果是质数返回1，否则返回0
static int is_prime(int num){
    int i = 0; // 循环变量
    sleep(1);  // 线程休眠1
    if(num < 2){
        return 0; // 不是质数
    }
    for(i = 2; i <= num / 2; i++){
        if(num % i == 0){
            return 0; // 不是质数
        }
    }
    return 1; // 是质数
}

// 每个线程都要做的事情: 循环抢任务
void *thr_job(void *arg){
    /**
     * 循环抢任务
     * [1]遇到job = 0，说明任务还没有发放，线程就等待
     * [2]遇到job > 0，说明任务已经发放，线程就处理任务
     * [3]遇到job = -1，说明所有任务发放完毕，于是应该退出线程
     */
    int n = 0; // 另存空间

    // 循环抢任务
    while(1){
        pthread_mutex_lock(&mutex); // 加锁(因为此时线程需要访问job变量，所以需要加锁，保证同一时刻只有一个线程访问job变量)
        // 如果job变量的值为0，说明没有任务，线程就等待
        if(job == 0){
            pthread_mutex_unlock(&mutex); // 解锁
            continue; // 继续抢任务，前往下一次循环
        }
        // 如果job变量的值为-1，说明所有任务发放完毕，线程就退出
        if(job == -1){
            pthread_mutex_unlock(&mutex); // 解锁
            pthread_exit(0); // 线程退出，返回NULL
        }

        // 抢到任务了，线程就处理任务
        n = job; // 另存空间，抢到了n这个任务
        job = 0; // 抢到任务之后，重置job变量的值为0，表示任务已经被抢走了

        pthread_mutex_unlock(&mutex); // 解锁

        // 如果是质数
        if(is_prime(n)){
            printf("%d Is A Prime Number!\n", n);
        }
    }
}

int main(int argc, char *argv[]){
    /**
     * [1]初始化互斥量(要么调用pthread_mutex_init()函数，要么使用PTHREAD_MUTEX_INITIALIZER宏来初始化互斥量)
     * [2]创建线程
     * [3]发放任务(即修改job变量的值)，发放完任务之后，修改job变量的值为-1，表示所有任务发放完毕
     * [4]告诉所有线程，所有任务已经发放完毕(即修改job变量的值为-1)，让所有线程都能退出
     * [5]等待线程退出(即调用pthread_join()函数)，回收线程
     * [6]销毁互斥量
     * 
     */
    pthread_t tid[NUM_THREADS]; // 存储新创建的4个线程的ID
    int ret = 0; // 存储函数调用的返回值
    int i = 0;   // 循环变量
    
    // [2]循环创建4个线程，每个线程都执行thr_job函数
    for(i = 0; i < NUM_THREADS; i++){
        ret = pthread_create(&tid[i], NULL, thr_job, NULL); // 创建线程，只要创建成功，立马执行thr_job函数
        // 如果 线程创建失败
        if (ret != 0) {
            fprintf(stderr, "pthread_create(): %s\n", strerror(ret)); // KILLME: strerror()函数
            exit(1); // 由于创建进程失败，退出程序，并返回错误码1
        }
    }

    // [3]发放任务
    for(i = MIN; i <= MAX; i++){
        // 由于主线程需要访问job变量，所以主线程要拿到锁
        pthread_mutex_lock(&mutex); // 主线程拿到锁
        // 主线程不断地去看job变量的值，如果job变量的值大于0，说明上一个任务还没有被取走，主线程就等待；如果job变量的值为0，说明上一个任务已经被取走了，主线程就发放新任务
        while(job > 0){
            pthread_mutex_unlock(&mutex); // 解锁
            pthread_mutex_lock(&mutex); // 加锁
        }
        // 既然跳出while循环了，说明上一个任务已经被取走了，主线程就发放新任务
        job = i; // 修改job变量的值为i，main线程发放新任务
        pthread_mutex_unlock(&mutex); // 主线程把锁归还(让其它线程去拿锁，看到job变量的值为i，从而抢任务)
    }

    // 到这里任务已经发放完毕了，但最后一个任务还有没有被取走还不知道，120-124行就是为了等待最后一个任务被取走
    pthread_mutex_lock(&mutex); // 主线程拿到锁
    // 然后不断地去看job变量的值，如果job变量的值大于0，说明最后一个任务还没有被取走，主线程就等待；如果job变量的值为0，说明最后一个任务已经被取走了，主线程就可以告诉所有线程，所有任务已经发放完毕了，让所有线程都能退出了
    while(job > 0){ // 只要job变量的值大于0，说明还有任务没有被取走，main线程就等待
        pthread_mutex_unlock(&mutex); // 解锁
        pthread_mutex_lock(&mutex); // 加锁(因为此时main线程需要访问job变量，所以需要加锁，保证同一时刻只有一个线程访问job变量)
    }

    // [4]告诉所有线程，所有任务已经发放完毕(即修改job变量的值为-1)，让所有线程都能退出
    job = -1; // 修改job变量的值为-1，表示所有任务发放完毕
    pthread_mutex_unlock(&mutex); // 这里为什么要解锁呢？因为上面while循环中，main线程是加锁的，所以这里需要解锁，才能让其它线程拿到锁，看到job变量的值为-1，才知道没有任务了，从而退出线程
    
    // [5]等待线程退出，回收线程
    for(i = 0; i < NUM_THREADS; i++){
        pthread_join(tid[i], NULL); // 等待线程退出，回收线程
    }

    // [6]销毁互斥量
    pthread_mutex_destroy(&mutex);

    return 0;
}