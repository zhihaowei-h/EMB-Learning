#include <stdio.h>
#include "fsm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    fsm_t *t9tot10 = NULL;
    fsm_t *t10tot9 = NULL;

    int fd9 = open("/dev/tty9", O_RDWR | O_NONBLOCK);
    if(fd9 == -1){
        perror("open()");
        return -1;
    }
    int fd10 = open("/dev/tty10", O_RDWR | O_NONBLOCK);
    if(fd10 == -1){
        perror("open()");
        close(fd9);
        return -2;
    }

    // 初始化
    fsm_init(&t9tot10, fd9, fd10);
    fsm_init(&t10tot9, fd10, fd9);
	write(fd9, "[****tty9****]", 14); // 先往tty9的页缓存中写入一些数据
	write(fd10, "[!!!tty10!!!]", 13); // 先往tty10的页缓存中写入一些数据


    while(t10tot9->state != STATE_T && t9tot10->state != STATE_T)
	{
		fsm_drive(t10tot9);//推动r9w10的有限状态机
		fsm_drive(t9tot10);//推动r10w9的有限状态机
	}

    // 
    fsm_destory(t10tot9);
    fsm_destory(t9tot10);

    return 0;
}