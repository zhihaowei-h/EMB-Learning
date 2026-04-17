/*===============================================
 *   文件名称：alarm_scheduler.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：闹钟调度器模块
 *            基于SIGALRM信号实现多任务定时调度
 ================================================*/

#include "../inc/iot_gateway.h"

// 定时任务链表头
static alarm_task_t *g_task_list = NULL;
static pthread_mutex_t g_alarm_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * 功能：SIGALRM信号处理函数
 * 参数：signo - 信号编号
 * 返回：无
 */
static void alarm_handler(int signo)
{
    alarm_task_t *task;
    
    (void)signo;  // 忽略未使用的参数警告
    
    pthread_mutex_lock(&g_alarm_mutex);
    
    // 遍历任务链表
    task = g_task_list;
    while (task != NULL) {
        task->countdown--;
        
        // 倒计时到0，执行回调
        if (task->countdown <= 0) {
            task->countdown = task->interval;  // 重置倒计时
            
            // 执行回调函数（注意：信号处理函数中应尽量少做事）
            if (task->callback != NULL) {
                task->callback();
            }
        }
        
        task = task->next;
    }
    
    pthread_mutex_unlock(&g_alarm_mutex);
    
    // 重新设置闹钟
    alarm(1);
}

/*
 * 功能：初始化闹钟调度器
 * 参数：无
 * 返回：无
 */
void alarm_scheduler_init(void)
{
    struct sigaction sa;
    
    // 设置信号处理函数
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        return;
    }
    
    // 启动第一个闹钟
    alarm(1);
    
    log_write(LOG_INFO, "Alarm scheduler initialized");
}

/*
 * 功能：添加定时任务
 * 参数：interval - 时间间隔（秒）
 *       callback - 回调函数
 * 返回：成功返回0，失败返回-1
 */
int alarm_scheduler_add(int interval, alarm_callback_t callback)
{
    alarm_task_t *task;
    
    if (interval <= 0 || callback == NULL) {
        return -1;
    }
    
    // 分配任务结构
    task = (alarm_task_t *)malloc(sizeof(alarm_task_t));
    if (task == NULL) {
        perror("malloc");
        return -1;
    }
    
    // 初始化任务
    task->interval = interval;
    task->countdown = interval;
    task->callback = callback;
    
    pthread_mutex_lock(&g_alarm_mutex);
    
    // 插入链表头
    task->next = g_task_list;
    g_task_list = task;
    
    pthread_mutex_unlock(&g_alarm_mutex);
    
    log_write(LOG_INFO, "Alarm task added: interval=%d seconds", interval);
    
    return 0;
}

/*
 * 功能：运行调度器（主要用于测试）
 * 参数：无
 * 返回：无
 */
void alarm_scheduler_run(void)
{
    // 实际上，调度器是由SIGALRM信号驱动的
    // 这个函数可以用于等待或其他操作
    pause();
}

/*
 * 功能：销毁闹钟调度器
 * 参数：无
 * 返回：无
 */
void alarm_scheduler_destroy(void)
{
    alarm_task_t *task, *next;
    
    // 取消闹钟
    alarm(0);
    
    pthread_mutex_lock(&g_alarm_mutex);
    
    // 释放所有任务
    task = g_task_list;
    while (task != NULL) {
        next = task->next;
        free(task);
        task = next;
    }
    
    g_task_list = NULL;
    
    pthread_mutex_unlock(&g_alarm_mutex);
    
    log_write(LOG_INFO, "Alarm scheduler destroyed");
}
