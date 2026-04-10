#include <stdio.h>
#include "anytimer.h"
#include <signal.h>
#include <unistd.h>
#define ANYTIMER_MAX 1024 // 最大支持1024个闹钟

typedef struct{
    int remain; // 闹钟的剩余秒数
    HANDLER callback; // 闹钟响了之后执行的行为(回调函数)
    void *arg;        // 回调函数的参数
    int is_used;      // 该闹钟的状态(1: 使用中 0: 空闲中)
}anytimer_st; // 闹钟结构

static anytimer_st timer_libs[ANYTIMER_MAX]; // 闹钟库。存储所有的闹钟实例
static int lib_inited = 0; // 记录闹钟库是否被初始化(1: 已被初始化 0: 未被初始化)

// SIGALRM信号的行为
static void alarm_handler(int none){
    int i = 0;
    alarm(1); // 设置1s的闹钟
    // 循环遍历: 更新所有闹钟的剩余秒数
    for(i = 0; i < ANYTIMER_MAX; i++){
        // 如果当前闹钟正在被使用
        if(timer_libs[i].is_used == 1){
            timer_libs[i].remain--; // 更新当前闹钟剩余秒数
            // 如果 闹钟到了定时时间
            if(timer_libs[i].remain <= 0){
                // 如果 该闹钟的回调函数不为空
                if(timer_libs[i].callback != NULL){
                    timer_libs[i].callback(timer_libs[i].arg);
                }
                timer_libs[i].is_used = 0; // 执行完回调函数后，将闹钟设置为空闲状态
            }
        }
    }
}

// √
static int get_anytimer_pos(void){
    int i = 0;

    for(i = 0; i < ANYTIMER_MAX; i++){
        if(timer_libs[i].is_used == 0){
            return i;
        }
    }
    return -1;
}

// 初始化闹钟库
static void lib_init(void){
    int i = 0;

    // 循环遍历: 初始化闹钟库中的所有闹钟
    for(i = 0; i < ANYTIMER_MAX; i++){
        timer_libs[i].remain = 0;
        timer_libs[i].callback = NULL;
        timer_libs[i].arg = NULL;
        timer_libs[i].is_used = 0;
    }

    signal(SIGALRM, alarm_handler);
    alarm(1);

    lib_inited = 1; // 切换闹钟库的初始化状态，即切换为"已初始化状态"
}

int anytimer_init(int seconds, HANDLER handler, void *arg){
    int pos = 0; // 当前闹钟库中可用的最小下标的闹钟的下标

    // 如果 参数不合理
    if(seconds <= 0 || handler == NULL){
        return -1;
    }

    // 如果 还没有初始化闹钟库
    if(!lib_inited){
        lib_init();
    }
    
    pos = get_anytimer_pos();  // 查找当前闹钟库中可用的最小下标的闹钟的下标

    // 如果 查找失败
    if(pos < 0){
        return -2;
    }
    timer_libs[pos].remain = seconds;   // 存储客户指定的秒数
    timer_libs[pos].callback = handler; // 存储客户指定的回调函数 
    timer_libs[pos].arg = arg;          // 存储客户指定回调函数的参数
    timer_libs[pos].is_used = 1;        // 切换pos闹钟的状态

    return pos;
}

// 销毁闹钟库中的timer_id闹钟
int anytimer_destory(int timer_id){
    // 如果 参数不合理
    if(timer_id < 0 || timer_id >= ANYTIMER_MAX){
        return -1;
    }

    // 如果 用户指定的闹钟未被使用
    if(timer_libs[timer_id].is_used == 0){
        return -2;
    }
    timer_libs[timer_id].remain = 0;
    timer_libs[timer_id].callback = NULL;
    timer_libs[timer_id].arg = NULL;
    timer_libs[timer_id].is_used = 0;

    return 0;
}