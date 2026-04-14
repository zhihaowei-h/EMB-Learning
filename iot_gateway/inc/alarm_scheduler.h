#ifndef ALARM_SCHEDULER_H
#define ALARM_SCHEDULER_H

#include "iot_gateway.h"

// 定时任务回调函数指针
typedef void (*timer_callback)(void *arg);

// 定时任务节点
typedef struct TimerTask {
    int interval;               // 触发间隔（秒）
    int time_left;              // 剩余倒计时
    timer_callback callback;    // 任务函数
    void *arg;                  // 函数参数
    struct TimerTask *next;     // 链表指针
} TimerTask;

// 初始化调度器
void init_alarm_scheduler();

// 注册定时任务 (interval: 秒)
int register_timer_task(int interval, timer_callback cb, void *arg);

// 核心调度逻辑（由信号触发）
void run_alarm_scheduler();

#endif // ALARM_SCHEDULER_H