#pragma once

// 状态机的4种状态
enum {
    STATE_R = 0,
    STATE_W,
    STATE_E,
    STATE_T
};

#define BUFSIZE 1024

// 状态机-结构体(想象成搬运工)
typedef struct{
    int rfd; // 货源地(要从哪儿搬)
    int wfd; // 目的地(货物的目的地)
    char buf[BUFSIZE]; // 搬运车
    int count; // 从货源地往搬运车上搬了多少(搬运车有多少)
    int pos; // 从搬运车往目的地搬了多少
    int state; 
    /*
    搬运工此时的状态
    STATE_R: 正在从货源地往搬运车上搬
    STATE_W: 正在从搬运车往目的地搬
    STATE_E: 搬运工出了事故
    STATE_T: 搬运工死了
    */ 
    char *msgerr; // 事故记录本
}fsm_t;

// 初始化有限状态机
int fsm_init(fsm_t **fsm, int rfd, int wfd); // 因为我要动态给状态机开辟空间，就是要修改状态机的地址，所以要传状态机的二级指针

// 驱动有限状态机
int fsm_drive(fsm_t *fsm); // 驱动状态机的过程中会对状态机本身修改，所以传其一级地址

// 销毁有限状态机
int fsm_destory(fsm_t *fsm); // 