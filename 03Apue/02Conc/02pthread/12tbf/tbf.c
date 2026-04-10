#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tbf.h"

#define TBFMAX 1024
 
typedef struct tbf_st{
    int token; // 表示当前令牌桶的令牌数
    int cps;   // 表示当前令牌桶的速率
    int burst; // 当前令牌桶的容量
#if defined(PTHREAD)
    pthread_mutex_t mutex; // 当前令牌桶的互斥锁，用于保护令牌桶中的共享资源
    pthread_cond_t cond;   // 当前令牌桶的条件变量，用于等待令牌桶积攒令牌
#endif
}tbf_t; // 令牌桶的数据结构
 
static tbf_t *tbf_libs[TBFMAX];  // 令牌桶库的定义
static int initd; // 作为是否启动令牌桶的标志(0: 空闲中 1: 工作中)

#if defined(PTHREAD)
static pthread_mutex_t mut_job = PTHREAD_MUTEX_INITIALIZER; // 初始化令牌桶库的互斥锁
static pthread_cond_t jobtid = PTHREAD_COND_INITIALIZER;    // 初始化令牌桶库的条件变量
#endif

// 条件编译，只有在定义了PTHREAD的情况下才会编译以下代码
#if defined(PTHREAD)
static void *thr_func(void *arg){
    int i = 0; // 循环变量
    // FIXME: 下面的代码存在一个问题：当主线程在发放任务时，子线程可能正在等待条件变量，此时主线程会一直持有mut_job锁，导致子线程无法获取mut_job锁，无法被唤醒，造成死锁。
    while(1){
        pthread_mutex_lock(&mut_job); // 拿令牌桶库的锁，保护令牌桶库中的共享资源(或叫临界资源)tbf_libs
        for(i = 0; i < TBFMAX; i++){
            // 如果 当前令牌桶库的下标i的令牌桶处于工作中
            if(tbf_libs[i] != NULL){
                pthread_mutex_lock(&tbf_libs[i]->mutex); // 拿当前令牌桶的锁，保护当前令牌桶中的共享资源(或叫临界资源)token
                tbf_libs[i]->token += tbf_libs[i]->cps; // 积攒令牌
                // 如果当前令牌桶中的令牌数超过该令牌桶的上限
                if(tbf_libs[i]->token > tbf_libs[i]->burst)
                    tbf_libs[i]->token = tbf_libs[i]->burst; // 把当前令牌桶中的令牌数设置为该令牌桶的上限
                pthread_mutex_unlock(&tbf_libs[i]->mutex); // 释放当前令牌的锁
                pthread_cond_signal(&tbf_libs[i]->cond); // 给当前令牌桶发送条件变量变化的通知
            }
        }
        pthread_mutex_unlock(&mut_job); // 释放令牌桶库的锁，允许其他线程访问共享资源
        sleep(1); // 线程休眠1秒(留一个取消点)
    }
}

static void module_unload(void){
    int i = 0; // 循环变量
    pthread_cancel(jobtid); // 取消上面创建的线程，停止令牌桶的工作
    pthread_join(jobtid, NULL); // 等待上面创建的线程退出，然后回收线程资源
    for(i = 0; i < TBFMAX; i++){
        // 如果 当前令牌桶库的下标i的令牌桶处于工作中
        if(tbf_libs[i] != NULL){
            free(tbf_libs[i]); // 销毁当前令牌桶
            tbf_libs[i] = NULL; // 避免出现野指针
        }
    }
}

// 启动模块
static void module_load(void){
    int ret = 0; // 存储函数调用的返回值
    ret = pthread_create(&jobtid, NULL, thr_func, NULL); // 创建线程，执行thr_job函数
    // 如果 线程创建失败
    if (ret != 0) {
        fprintf(stderr, "pthread_create(): %s\n", strerror(ret)); // 输出错误信息
        exit(1); // 由于创建线程失败，退出程序，并返回错误码1
    }

    atexit(module_unload); // 注册模块卸载函数，在程序退出时调用
}

#else
// SIGALRM信号的行为
static void alarm_handler(int none){
    int i = 0; // 循环变量
    alarm(1);
 
    // 遍历所有令牌桶
    for(i = 0; i < TBFMAX; i++){
        // 如果当前令牌桶正在工作中
        if(tbf_libs[i] != NULL){
            tbf_libs[i]->token += tbf_libs[i]->cps; // 给当前令牌桶积攒该令牌桶指定的令牌
 
            // 如果当前令牌桶的令牌数超过该令牌桶的上限
            if(tbf_libs[i]->token > tbf_libs[i]->burst)
                tbf_libs[i]->token = tbf_libs[i]->burst; // 把当前令牌桶中令牌数设置为该令牌桶指定的上限
        }
    }
}
 
// 循环遍历令牌桶库中当前可用的最小下标的令牌桶的下标
static int get_tbf_pos(void){
    int i = 0;
 
    for(i = 0; i < TBFMAX; i++){
        // 如果 当前位置可用
        if(tbf_libs[i] == NULL) return i;
    }
    return -1; // 没有找到合适的位置
}
 
// 启动模块
static void module_load(void){
    signal(SIGALRM, alarm_handler); //
    alarm(1);
}
 
#endif
// 初始化令牌桶
int tbf_init(int cps, int burst){
 
    int pos = 0; // 用来存储查询到的当前可用的下标最小的令牌桶库的下标
 
    // 如果 形参不合理
    if(cps <=  0 || burst <= 0) return -1;
 
    // 如果 令牌桶处于空闲中
    if(!initd){
        module_load(); // 让令牌桶处于 工作中
        initd = 1;     // 切换状态为 工作中
    }
 
    pos = get_tbf_pos(); // 获取当前可用的下标最小的令牌桶库的下标
 
    // 如果 没有找到合适位置
    if(pos == -1){
        return -2;
    }
 
    tbf_libs[pos] = malloc(sizeof(TBFMAX)); // 为令牌桶开辟空间
    // 如果令牌桶空间开辟失败
    if(tbf_libs[pos] == NULL){
        return -3;
    }
 
    tbf_libs[pos]->cps = cps; // 存储客户指定的速率
    tbf_libs[pos]->burst = burst;// 存储客户指定的上限
    tbf_libs[pos]->token = 0;// 初始化令牌桶中的令牌数为0
#ifdef PTHREAD
    pthread_mutex_init(&tbf_libs[pos]->mutex, NULL); // 初始化令牌桶的互斥锁
    pthread_cond_init(&tbf_libs[pos]->cond, NULL);   // 初始化令牌桶的条件变量
#endif
 
    return pos; // 返回当前初始化成功的令牌桶下标
}

#ifdef PTHREAD
static int getmin(int m, int n){
    return m < n ? m : n;
}
#endif

// 取走td令牌桶的n个令牌数
int tbf_fetch_token(int td, int n){
    int fetch_token = 0; // 取走的令牌数
    // 如果 参数不合理
    if(td < 0 || td >= TBFMAX || n <= 0) return -1;
 
    // 如果 下标为td的令牌桶 不存在
    if(tbf_libs[td] == NULL) return -2;
 
#ifdef PTHREAD
    pthread_mutex_lock(&tbf_libs[td]->mutex); // 拿td令牌桶的锁，保护td令牌桶中的共享资源(或叫临界资源)token
    
    // 等待td令牌桶积攒令牌
    while(tbf_libs[td]->token <= 0)
        pthread_cond_wait(&tbf_libs[td]->cond, &tbf_libs[td]->mutex); // 如果td令牌桶中的令牌数不大于0，说明td令牌桶还没有积攒出令牌，解锁并继续等待

    // 如果td令牌桶的令牌充足
    if(tbf_libs[td]->token >= n){
        fetch_token = n; // 取走客户指定的令牌数
    }else{ // 如果td令牌桶的令牌不充足
        fetch_token = tbf_libs[td]->token; // 有多少取多少
    }

    tbf_libs[td]->token -= fetch_token; // 更新td令牌桶中的令牌数
    
    pthread_mutex_unlock(&tbf_libs[td]->mutex); // 释放td令牌桶的锁，允许其他线程访问共享资源
    
    return fetch_token; // 返回成功取走的令牌数

#else

    // 等待td令牌桶积攒令牌
    while(tbf_libs[td]->token <= 0){
        pause();
    }
 
    // 如果td令牌桶的令牌充足
    if(tbf_libs[td]->token >= n){
        fetch_token = n; // 取走客户指定的令牌数
    }else{ // 如果td令牌桶的令牌不充足
        fetch_token = tbf_libs[td]->token; // 有多少取多少
    }

    tbf_libs[td]->token -= fetch_token; // 更新td令牌桶中的令牌数
    
#endif

    return fetch_token; // 返回成功取走的令牌数
}
 
// 销毁下标为td的令牌桶
int tbf_destroy(int td){
    // 如果 参数不合理
    if(td < 0 || td >= TBFMAX) return -1;
 
    // 如果 要销毁的令牌库不存在
    if(tbf_libs[td] == NULL) return -2;

#ifdef PTHREAD
    pthread_mutex_destroy(&tbf_libs[td]->mutex); // 销毁互斥锁

    pthread_cond_destroy(&tbf_libs[td]->cond);   // 销毁条件变量
#endif
 
    free(tbf_libs[td]); // 释放td令牌库
 
    tbf_libs[td] = NULL; // 避免出现野指针
 
    return 0; // 销毁成功
}

#ifdef PTHREAD
// 返还令牌
int tbf_return_token(int td, int ntoken){
    // 如果 参数不合理
    if(td < 0 || td >= TBFMAX || ntoken <= 0) return -1;
    
    // 如果 要返还令牌的令牌库不存在
    if(tbf_libs[td] == NULL) return -2;

    pthread_mutex_lock(&tbf_libs[td]->mutex); // 拿td令牌桶的锁，保护td令牌桶中的共享资源(或叫临界资源)token
    
    tbf_libs[td]->token += ntoken; // 给td令牌桶返还ntoken个令牌

    // 如果td令牌桶中的令牌数超过该令牌桶的上限
    if(tbf_libs[td]->token > tbf_libs[td]->burst)
        tbf_libs[td]->token = tbf_libs[td]->burst; // 把td令牌桶中的令牌数设置为该令牌桶的上限

    pthread_mutex_unlock(&tbf_libs[td]->mutex); // 释放td令牌桶的锁，允许其他线程访问共享资源
    
    pthread_cond_broadcast(&tbf_libs[td]->cond); // 给td令牌桶发送条件变量变化的通知
    
    return 0;
}

// 销毁所有令牌桶
void tbf_destroy_all(void){
    int i = 0; // 循环变量
    pthread_mutex_lock(&mut_job); // 拿令牌桶库的锁，保护令牌桶库中的共享资源(或叫临界资源)tbf_libs
    for(i = 0; i < TBFMAX; i++){
        // 如果 当前令牌桶库的下标i的令牌桶处于工作中
        if(tbf_libs[i] != NULL) tbf_destroy(i); // 销毁当前令牌桶
    }
    pthread_mutex_unlock(&mut_job); // 释放令牌桶库的锁
}
#endif