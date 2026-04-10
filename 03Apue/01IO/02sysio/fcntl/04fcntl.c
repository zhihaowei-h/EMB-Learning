// 演示fcntl(2)的功能4: 获得/设置异步I/O所有权(cmd = F_GETOWN或F_SETOWN)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

// 信号处理函数：当有数据可读时，内核会调用这个函数
void sigio_handler(int signum) {
    char buffer[1024];
    // 非阻塞读取数据
    int n = read(0, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("\n[异步中断触发!] 收到你的输入: %s", buffer);
    }
}

int main() {
    printf("异步 I/O 演示启动。程序主循环在忙碌，你可以随时敲击键盘输入内容...\n");

    // 1. 注册 SIGIO 信号处理函数
    signal(SIGIO, sigio_handler);

    /**
     * F_SETOWN: 表示要设置异步I/O所有者(大白话就是当有数据可读时，内核会向这个所有者发送 SIGIO 信号)  (只要开启了异步 I/O，一旦数据就绪，内核默认且只会发送 SIGIO 信号)
     * getpid(): 获取当前进程的PID，表示把当前进程设置为标准输入(文件描述符0)的异步I/O所有者
     */
    fcntl(0, F_SETOWN, getpid());

    /**
     * 给指向标准输入的文件描述符开启 异步模式 (O_ASYNC) 和 非阻塞模式 (O_NONBLOCK)
     * O_ASYNC: 让内核在标准输入有数据可读时，向设置的所有者发送 SIGIO 信号
     * O_NONBLOCK: 设置非阻塞模式，让 read() 调用在没有数据可读时不会阻塞,而是去干其他事情(比如继续执行下面的模拟耗时任务)，等到有数据可读时，内核会通过 SIGIO 信号通知我们，然后我们在信号处理函数里去读数据
     */
    int flags = fcntl(0, F_GETFL); // 获取当前标准输入文件描述符(0)的文件状态标志
    flags |= O_ASYNC | O_NONBLOCK; // 在原有标志基础上，追加 O_ASYNC 和 O_NONBLOCK 标志
    fcntl(0, F_SETFL, flags);

    // 4. 模拟程序正在做其他极其耗时的任务
    while (1) {
        printf(".");
        fflush(stdout); // FIXME刷新输出缓冲区
        sleep(1);       // 假装在忙碌计算
        // 在这个过程中，如果你在键盘输入内容并按回车，内核会检测到标准输入有数据可读，然后向我们发送 SIGIO 信号，触发 sigio_handler 函数的执行，在那里我们就能读取并打印出你输入的内容了
    }

    return 0;
}