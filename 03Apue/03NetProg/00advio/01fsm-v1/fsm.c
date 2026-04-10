#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "fsm.h"

// 状态机初始化函数，设置读写文件描述符和初始状态
int fsm_init(fsm_t **f, int rfd, int wfd) {
    fsm_t *me = malloc(sizeof(fsm_t)); // 指向开辟的空间
    // 如果给状态机分配内存失败
    if (me == NULL) return -1;

    me->rfd = rfd; // 读文件描述符
    me->wfd = wfd; // 写文件描述符
    memset(me->buf, 0, BUFSIZE); // 初始化BUFFER空间 // FIXME: memset
    me->state = STATE_R; // 初始状态为读状态  
    me->count = 0;       // 把成功读取的字节数清0
    me->pos = 0;         // 把成功写入的字节个数清0
    me->errmsg = NULL;   // 错误信息初始化为NULL

    // 关键：强制设置为非阻塞模式
    int r_flags = fcntl(rfd, F_GETFL); // 获取文件描述符rfd打开文件的文件状态标志
    r_flags |= O_NONBLOCK; // 加上非阻塞标志
    fcntl(rfd, F_SETFL, r_flags); // 设置读文件描述符为非阻塞模式
    
    int w_flags = fcntl(wfd, F_GETFL); // 获取文件描述符wfd打开文件的文件状态标志
    w_flags |= O_NONBLOCK; // 加上非阻塞标志
    fcntl(wfd, F_SETFL, w_flags); // 设置写文件描述符为非阻塞模式

    *f = me; // 把开辟的空间地址回填给参数f

    return 0;
}

// 状态机驱动函数，根据当前状态执行相应的操作
void fsm_drive(fsm_t *f) {
    ssize_t n;

    // 根据状态机当前的状态执行相应的操作
    switch (f->state) {
        case STATE_R: // 读状态：尝试从rfd读取数据到buf中
            f->count = read(f->rfd, f->buf, BUFSIZE);
            if (f->count > 0) {
                f->pos = 0; // 续写位置重置为0
                f->state = STATE_W;
            } else if (f->count == 0) { // FIXME如果读到EOF，说明读完了? 任务要结束？
                f->state = STATE_T;
            } else if(f->count == -1) {
                // 如果 不是假错
                if (errno != EAGAIN) {
                    f->errmsg = "read()"; // 记录出错函数的函数名
                    f->state = STATE_E;
                }
                // 如果是EAGAIN，状态保持STATE_R，下次drive继续读
            }
            break;

        case STATE_W: // FIXME写状态：尝试从buf中写数据到wfd
            // 续写逻辑：从 pos 位置开始写，剩余长度为 count - pos
            int ret = write(f->wfd, f->buf + f->pos, f->count - f->pos);
            if (ret == -1) {
                if (errno != EAGAIN) {
                    f->errmsg = "write()";
                    f->state = STATE_E;
                }
            } else {
                if (ret < f->count) {
                    f->pos += ret; // 更新续写位置
                    f->count -= ret; // 更新剩余长度
                }else{
                    f->state = STATE_R; // 全部写完，回到读状态
                }
                // 如果是EAGAIN，状态保持STATE_W，下次drive继续续写
            }
            break;

        case STATE_E:
            perror(f->errmsg);
            f->state = STATE_T;
            break;

        case STATE_T:
            // 终止态，不做事，等待销毁
            break;
        default:
            // 不可能到达这里
            break;
    }
    return;
}

// 状态机销毁函数，释放状态机占用的资源
int fsm_destroy(fsm_t *f) {
    if (f != NULL) free(f);
    f = NULL; // 避免野指针
    return 0;
}