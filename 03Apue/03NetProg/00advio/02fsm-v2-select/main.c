#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h> // 引入 select 所需的头文件
#include <sys/time.h>
#include "fsm.h"
#include <errno.h>
#include <sys/select.h>

#define SELECT

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

#ifdef SELECT
    fd_set rset, wset; // 定义读写监听集合

#endif

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

    // 推动有限状态机工作
    while (fsm12->state != STATE_T && fsm21->state != STATE_T) {
        // 不再是无脑地推动了，而是需要一定条件才推动

        // 如果r9w10的状态机处于E态
        if(fsm12->state == STATE_E){
            fsm_drive(fsm12); // 推动状态机，让它到达T态
            continue;
        }
        // 如果r10w9的状态机处于E态
        if(fsm21->state == STATE_E){
            fsm_drive(fsm21); // 推动状态机，让它到达T态
            continue;
        }

        // 初始化集合
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        // 根据状态机的状态决定监听哪个文件描述符的读写事件
        if(fsm12->state == STATE_R){
            FD_SET(fsm12->rfd, &rset); // 监听r9的读事件
        }else if(fsm12->state == STATE_W){
            FD_SET(fsm12->wfd, &wset); // 监听w10的写事件
        }
        if(fsm21->state == STATE_R){
            FD_SET(fsm21->rfd, &rset); // 监听r10的读事件
        }else if(fsm21->state == STATE_W){
            FD_SET(fsm21->wfd, &wset); // 监听w9的写事件
        }

        // 调用select监听事件
        int ret = select(maxfd + 1, &rset, &wset, NULL, NULL);
        if (ret == -1) {
            if (errno == EINTR) {
                continue; // 如果被信号中断，继续监听
            }
            perror("select");
            exit(1);
        }
        // 根据监听结果推动状态机
        if (ret > 0) {
            if (FD_ISSET(fsm12->rfd, &rset) || FD_ISSET(fsm12->wfd, &wset)) {
                fsm_drive(fsm12); // 推动r9w10的状态机
            }
            if (FD_ISSET(fsm21->rfd, &rset) || FD_ISSET(fsm21->wfd, &wset)) {
                fsm_drive(fsm21); // 推动r10w9的状态机
            }
        }
    }

    fsm_destroy(fsm12);
    fsm_destroy(fsm21);
    close(fd1);
    close(fd2);

    return 0;
}