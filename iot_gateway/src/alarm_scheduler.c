// 1.3.6 流量控制与定时任务需求
#include "alarm_scheduler.h"
#include <sys/time.h>

static TimerTask *g_timer_list = NULL;
static pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_alarm_scheduler() {
    struct itimerval itv;

    // 设定每秒触发一次 SIGALRM 信号 [cite: 70]
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
        LOG_ERROR("setitimer failed: %s", strerror(errno));
    } else {
        LOG_INFO("Alarm scheduler initialized (1Hz).");
    }
}

int register_timer_task(int interval, timer_callback cb, void *arg) {
    TimerTask *new_node = (TimerTask *)malloc(sizeof(TimerTask));
    if (!new_node) return -1;

    new_node->interval = interval;
    new_node->time_left = interval;
    new_node->callback = cb;
    new_node->arg = arg;

    pthread_mutex_lock(&timer_mutex);
    new_node->next = g_timer_list;
    g_timer_list = new_node;
    pthread_mutex_unlock(&timer_mutex);

    LOG_DEBUG("New timer task registered (Interval: %ds)", interval);
    return 0;
}

// 遍历任务链表并执行到期任务 [cite: 70]
void run_alarm_scheduler() {
    pthread_mutex_lock(&timer_mutex);
    TimerTask *curr = g_timer_list;
    while (curr) {
        curr->time_left--;
        if (curr->time_left <= 0) {
            // 时间到，执行回调
            if (curr->callback) {
                curr->callback(curr->arg);
            }
            // 重置倒计时
            curr->time_left = curr->interval;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&timer_mutex);
}