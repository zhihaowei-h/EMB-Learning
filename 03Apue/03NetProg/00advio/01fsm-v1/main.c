#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "fsm.h"
#define TTY1 "/dev/tty9"
#define TTY2 "/dev/tty10"

// 辅助函数：求两个数的最大值
int max(int a, int b) {
    return a > b ? a : b;
}

int main() {
    int fd1, fd2;
    fsm_t *fsm12 = NULL; // 指向状态机实例的一级指针：该状态机实例负责单向驱动 "读取 tty9 并写入 tty10" 的数据流转
    fsm_t *fsm21 = NULL; // 指向状态机实例的一级指针：该状态机实例负责单向驱动 "读取 tty10 并写入 tty9" 的数据流转

    fd1 = open(TTY1, O_RDWR | O_NONBLOCK); // 打开文件/dev/tty9，返回指向该文件的描述符，通过该文件描述符可读可写该文件，且非阻塞
    // 如果 打开文件tty9失败
    if (fd1 == -1) {
        perror("open tty9");
        exit(1);
    }

    write(fd1, "[****tty9****]", 14); // 通过文件描述符fd1往tty9写点东西，方便测试

    fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
    // 如果 打开文件tty10失败
    if (fd2 == -1) { 
        perror("open tty10");
        close(fd1);
        exit(1);
    }

    write(fd2, "[****tty10****]", 15); // 通过文件描述符fd2往tty10写点东西，方便测试

    fsm_init(&fsm12, fd1, fd2); // 初始化fsm12状态机
    fsm_init(&fsm21, fd2, fd1); // 初始化fsm21状态机

    // 引擎循环：如果这两个状态机不是都处于终止状态，就持续驱动
    while (fsm12->state != STATE_T && fsm21->state != STATE_T) {
        fsm_drive(fsm12);
        fsm_drive(fsm21);
    }

    // 清理资源：销毁状态机，关闭文件描述符
    fsm_destroy(fsm12);
    fsm_destroy(fsm21);
    close(fd1);
    close(fd2);

    return 0;
}