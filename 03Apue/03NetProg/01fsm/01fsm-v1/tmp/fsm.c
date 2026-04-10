#include "fsm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// 初始化有限状态机
int fsm_init(fsm_t **fsm, int rfd, int wfd){
    int rfd_fp_state = 0; // 读文件的文件状态标识
    int wfd_fp_state = 0; // 写文件的文件状态标识

    *fsm = malloc(sizeof(fsm));
    // 如果给状态机开辟空间失败
    if(fsm == NULL){
        perror("malloc()");
        return -1;
    }

    // 初始化状态机
    (*fsm)->rfd = rfd;
    (*fsm)->wfd = wfd;
    memset(*fsm, 0, BUFSIZE);
    (*fsm)->count = 0;
    (*fsm)->pos = 0;
    (*fsm)->state = STATE_R;
    (*fsm)->msgerr = NULL;

    // 强制设置非阻塞
    rfd_fp_state = fcntl(rfd, F_GETFL);
    fcntl(rfd, rfd_fp_state | O_NONBLOCK);

    wfd_fp_state = fcntl(wfd, F_GETFL);
    fcntl(wfd, wfd_fp_state | O_NONBLOCK);

    return 0;
}

// 驱动有限状态机
void fsm_drive(fsm_t *fsm){
    switch(fsm->state){
        case STATE_R:  // 搬运工处STATE_R状态时应该怎么操作
            int ret = read(fsm->rfd, fsm->buf, BUFSIZE); // 如果这里读的时候rfd里面还没有数据怎么办
            // rfd的文件流被关闭了，也就是不让往rfd的缓冲区中写入数据了
            if(ret == -1){
                if(errno != EAGAIN){
                    fsm->msgerr = "read()";
                    fsm->state = STATE_E;
                }
            }else if(ret == 0){
                fsm->state = STATE_T; // 货源地都没有了，搬运工可以滚了
            }else{
                // 此时fsm->buf里面应该是有数据的,可以开始写了
                fsm->count = ret;
                fsm->state = STATE_W;
            }
            break;
        case STATE_W:
            int ret = write(fsm->rfd, fsm->buf + fsm->pos, BUFSIZE);
            // rfd的文件流被关闭了，也就是不让往rfd的缓冲区中写入数据了
            if(ret == -1){
                fsm->state = STATE_T; // 货源地都没有了，搬运工可以滚了
            }else if(ret == 0){
                fsm->state = STATE_T; // 货源地都没有了，搬运工可以滚了
            }else{
                // 此时fsm->buf里面应该是有数据的
            }
            
        case STATE_E: 
        case STATE_T: 
        default: break;
    }
}

// 销毁有限状态机
int fsm_destory(fsm_t *fsm){

}