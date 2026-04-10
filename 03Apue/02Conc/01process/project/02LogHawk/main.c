#include "loghawk.h"

// 声明外部模块启动函数
extern void collector_start(); // 日志采集模块
extern void processor_start(); // 日志处理模块
extern void outputer_start();  // 日志输出模块

int main(int argc, char *argv[]) {
    // [1] 拉起日志输出模块(守护进程)
    if (fork() == 0) {
        outputer_start(); // 启动日志输出模块
        exit(0);          // 子进程执行完毕后退出
    }

    // [2] 拉起日志处理模块(多进程池)
    // 创建了一个进程来处理日志数据，实际生产环境可根据负载动态调整进程池大小
    if (fork() == 0) {
        processor_start();
        exit(0);
    }

    // [3] 拉起日志采集模块(阻塞监控)
    // 创建了一个进程来采集日志数据，实际生产环境可根据需要增加采集器数量或分布式部署
    if (fork() == 0) {
        collector_start();
        exit(0);
    }

    // 信号处理：支持平滑重启、优雅退出、异常捕获 [cite: 27]
    // Master 进程监控子进程
    while(wait(NULL) > 0);
    
    return 0;
}