#include "fsm.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// 初始化状态机
int fsm_init(fsm_t **f, int rfd, int wfd){
    *f = malloc(sizeof(fsm_t));

    (*f)->rfd = rfd;
    (*f)->wfd = wfd;
    memset(((*f)->buf), 0, BUFSIZE);
    (*f)->count = 0;
    (*f)->pos = 0;
    (*f)->state = STATE_R;
    (*f)->errmsg = NULL;

    // 强制设置 rfd 和 wfd 为非阻塞模式
    int rfd_state_flg = fcntl(rfd, F_GETFL); // 获取rfd的文件状态标志(也就是rfd访问所指文件的方式)
    fcntl(rfd, F_SETFL, rfd_state_flg | O_NONBLOCK); // 设置rfd设置为非阻塞
    int wfd_state_flg = fcntl(wfd, F_GETFL); // 获取wfd的文件状态标志(也就是wfd访问所指文件的方式)
    fcntl(wfd, F_SETFL, wfd_state_flg | O_NONBLOCK); // 设置wfd设置为非阻塞

    return 0;
}

// 驱动状态机
void fsm_drive(fsm_t *f){

    switch(f->state){
        case STATE_R:  // 状态机此时处于读状态(搬运工现在应该把货源地的数据搬运到车上)
            int count = read(f->rfd, f->buf, BUFSIZE);
            // 如果read()返回-1
            if(f->count == -1){
                // 如果errno == EAGAIN，是假错，说明读端没有数据导致没有读到数据，需要重新读 
                if(errno == EAGAIN){
                    break;
                }else{ // 读的过程中发生了错误
                    f->errmsg = "read()";
                    f->state = STATE_T;
                }
            }else if(f->count == 0){ // 读端关闭了或者彻底把读端读完了
                f->state = STATE_E;
            }
            break;

            // FIXME : 到此

        case STATE_W:
            

        break;


        case STATE_E:

        break;

        case STATE_T:

        break;

        default:break;
    }
}

// 销毁状态机
int fsm_destory(fsm_t *f){


}