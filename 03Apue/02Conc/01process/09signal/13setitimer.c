#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

// 信号处理函数：每次闹钟响了就执行这个
void sig_handler(int sig) {
    write(1, "!", 1);
}

int main() {
    struct sigaction sa;     // 信号处理方案sa
    struct itimerval timer;  // 时间结构timer

    // 1. 初始化信号处理方案
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL); // 捕获闹钟信号应该采取sa方案

    // 2. 配置定时器参数
    // it_value: 第一发什么时候打
    timer.it_value.tv_sec = 1;   // 1秒后打第一炮(不是已经设置过秒了吗，为啥还要再设置一遍微妙，多此一举吗？在 Linux 编程中，如果你只用“秒（seconds）”来计时，那程序会显得非常粗糙。struct timeval 这种“秒 + 微秒”的双成员设计，本质上是为了让你能够拼凑出带小数点的秒数。)
    timer.it_value.tv_usec = 0;

    // it_interval: 之后多久打一发
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 500000; // 每 500ms (0.5秒) 响一次

    // 3. 开启定时器
    // ITIMER_REAL: 按真实时间计时，时间到发送SIGALRM信号
    setitimer(ITIMER_REAL, &timer, NULL);

    // 4. 让主程序一直运行，观察效果
    while(1) {
        pause(); // 挂起等待信号，醒来后继续循环
    }

    return 0;
}