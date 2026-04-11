#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "fsm.h"

#define SELECT

#define TTY1    "/dev/tty9"
#define TTY2    "/dev/tty10"

int main(void){
    fsm_t *fsm12 = NULL; // r9w10的有限状态机
    fsm_t *fsm21 = NULL; // r10w9的有限状态机
    int fd1, fd2; // 存储打开文件的文件描述符
    fd_set rset, wset; // 定义读集和写集
    fd1 = open(TTY1, O_RDWR); // 打开/dev/tty9设备(打开时没有加入非阻塞选项)
    // 判断打开/dev/tty9设备是否失败
    if(fd1 == -1){
        perror("open()");
        return -1;
    }
    write(fd1, "[****tty9****]", 14);//用来区分是/dev/tty9设备
    fd2 = open(TTY2, O_RDWR | O_NONBLOCK);//打开/dev/tty10设备(打开时加入非阻塞选项)
    // 判断打开/dev/tty10设备是否失败
    if(fd2 == -1){
        perror("open()");
        close(fd1);
        return -2;
    }
    write(fd2, "[!!!tty10!!!]", 13);//用来区分是/dev/tty10设备

    fsm_init(&fsm12, fd1, fd2);//初始化r9w10的有限状态机
    fsm_init(&fsm21, fd2, fd1);//初始化r10w9的有限状态机

    //推动有限状态机的运行(不再是无脑的推动状态机,而是满足一定条件才推动状态机)
    while(fsm12->state != STATE_T && fsm21->state != STATE_T){
        // 如果状态机r9w10是否到达E态
        if(fsm12->state == STATE_E){
            fsm_drive(fsm12); // 推动r9w10状态机达到T态
            continue; // 结束本次循环,继续下一次循环
        }
        // 判断状态机r10w9是否到达E态
        if(fsm21->state == STATE_E){
            fsm_drive(fsm21);// 推动r10w9状态机达到T态
            continue;// 结束本次循环,继续下一次循环
        }

        // 初始化集合
        FD_ZERO(&rset); // 初始化读集为空
        FD_ZERO(&wset); // 初始化写集为空

        // 如果状态机fsm12是否到达R态(表示状态机fsm12准备要读数据了，所以要监听读文件描述符是否处于ready状态)
        if(fsm12->state == STATE_R)
            FD_SET(fsm12->rfd, &rset);//把读文件描述符添加到读集中
        else if(fsm12->state == STATE_W)//判断r9w10是否达到W态
            FD_SET(fsm12->wfd, &wset);//把写文件描述符添加到写集中

        // 判断状态机fsm21要监听的状态
        if(fsm21->state == STATE_R)//判断r10w9是否到达R态
            FD_SET(fsm21->rfd, &rset);//把读文件描述符添加到读集中
        else if(fsm21->state == STATE_W)//判断r10w9是否达到W态
            FD_SET(fsm21->wfd, &wset);//把写文件描述符添加到写集中

        // 开始监听(等待集合中的文件描述符变为"ready"态)
        // 注意: 阻塞的系统调用有可能被信号打断,所以加个判断
        if(select((fd1 > fd2 ? fd1 : fd2) + 1, &rset, &wset, NULL, NULL) == -1){
            // 如果监听被信号打断了,就继续监听; 否则就打印错误信息,并跳转到ERR_1的位置
            if(errno == EINTR) continue; // 结束本次循环,继续下一次循环
            perror("select()");
            goto ERR_1;//由于监听失败，所以跳转到ERR_1的位置，进行资源释放
        }

        //[7]判断哪个文件描述符处于ready状态
        if(FD_ISSET(fsm12->rfd, &rset) || FD_ISSET(fsm12->wfd, &wset)) // 判断fsm12的读文件描述符是否在读集中 || 判断fsm12的写文件描述符是否在写集中
            fsm_drive(fsm12); // 推动r9w10的有限状态机
        if(FD_ISSET(fsm21->rfd, &rset) || FD_ISSET(fsm21->wfd, &wset)) // 判断fsm21的读文件描述符是否在读集中 || 判断fsm21的写文件描述符是否在写集中
            fsm_drive(fsm21); // 推动r10w9的有限状态机
    }
ERR_1:
    fsm_destroy(fsm12);//释放r9w10的有限状态机
    fsm_destroy(fsm21);//释放r10w9的有限状态机

    close(fd2);//关闭/dev/tty10设备文件
    close(fd1);//关闭/dev/tty9设备文件

    return 0;
}

