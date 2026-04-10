#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h> // 引入 select 所需的头文件
#include <sys/time.h>
#include "fsm.h"
#include <errno.h>

#define TTY1 "/dev/tty9"
#define TTY2 "/dev/tty10"

// 辅助函数：求两个数的最大值
int max(int a, int b) {
    return a > b ? a : b;
}

int main() {
    int fd1, fd2;
    fsm_t *fsm12 = NULL; // r9w10的状态机
    fsm_t *fsm21 = NULL; // r10w9的状态机

    fd1 = open(TTY1, O_RDWR | O_NONBLOCK);
    if (fd1 == -1) {
        perror("open tty1");
        exit(1);
    }
    write(fd1, "[****tty9****]", 14); // 向tty9写点东西，方便测试

    fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
    if (fd2 == -1) { 
        perror("open tty2");
        close(fd1);
        exit(1);
    }
    write(fd2, "[****tty10****]", 15); // 向tty10写点东西，方便测试

    fsm_init(&fsm12, fd1, fd2); // 初始化状态机，设置读写文件描述符
    fsm_init(&fsm21, fd2, fd1); // 初始化状态机，设置读写文件描述符

    // 获取最大的文件描述符，这是 select 函数的第一个参数要求的
    int maxfd = max(fd1, fd2);

    // 引擎循环
    while (fsm12->state != STATE_T || fsm21->state != STATE_T) {
        fd_set rset, wset;
        FD_ZERO(&rset); // 清空读集合
        FD_ZERO(&wset); // 清空写集合

        // 1. 布置监听任务：看看状态机现在想要干什么？
        // 如果 fsm12 想读数据，就把它的读文件描述符加入读监听集合
        if (fsm12->state == STATE_R) {
            FD_SET(fsm12->rfd, &rset);
        }
        // 如果 fsm12 想写数据，就把它的写文件描述符加入写监听集合
        if (fsm12->state == STATE_W) {
            FD_SET(fsm12->wfd, &wset);
        }

        // 对 fsm21 做同样的事情
        if (fsm21->state == STATE_R) {
            FD_SET(fsm21->rfd, &rset);
        }
        if (fsm21->state == STATE_W) {
            FD_SET(fsm21->wfd, &wset);
        }

        // 2. 召唤前台秘书 select：程序在此处挂起休眠，CPU占用率降为 0%！
        // 只有当上述加入集合的 fd 有动作时，或者发生错误时，才会继续往下走
        if (select(maxfd + 1, &rset, &wset, NULL, NULL) < 0) {
            if (errno == EINTR) {
                continue; // 被信号打断是正常现象，继续轮询
            }
            perror("select()");
            break;
        }

        // 3. 被唤醒了！说明有事情可以做了，推动状态机运转
        // 这里可以直接无脑推动，因为 fsm_drive 里面是非阻塞的，
        // 即便不是它期待的数据，最多也就是返回 EAGAIN，不会卡死。
        if (fsm12->state != STATE_T) fsm_drive(fsm12);
        if (fsm21->state != STATE_T) fsm_drive(fsm21);
    }

    fsm_destroy(fsm12);
    fsm_destroy(fsm21);
    close(fd1);
    close(fd2);

    return 0;
}