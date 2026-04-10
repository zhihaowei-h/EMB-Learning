#pragma once

// 状态机的4种状态(未来状态机就是不断地在这4种状态之间切换)
enum {
    STATE_R = 0, // 读状态
    STATE_W,     // 写状态 
    STATE_E,     // 异常状态 
    STATE_T,     // 终止状态 
};

#define BUFSIZE 1024

// 有限状态机结构体(数据搬运工的"工作档案")
typedef struct {
    int rfd;       // 货源地：从哪个文件/设备里往外掏数据(Read FD)
    int wfd;       // 目的地：要把数据塞进哪个文件/设备(Write FD)
    
    char buf[BUFSIZE]; // 搬运小推车：一块用来暂存数据的内存空间
    
    int count;     // 车上装了多少货：这一趟总共成功"读"到了多少个字节
    int pos;       // 卸了多少货：这一趟已经成功"写"进去了多少个字节
                   // （注：在非阻塞模式下，货可能一次卸不完。用 count 减去 pos，就知道推车里还剩多少货没卸）
                   
    int state;     // 当前档位：搬运工现在处于什么状态？
                   // (是在专心装货 STATE_R，还是在努力卸货 STATE_W，或者是出了事故 STATE_E，还是已经下班 STATE_T）
                   
    char *errmsg;  // 事故记录本：如果发生了真正的系统级错误（非假错），记录是哪个动作（如 "read()"）引发的故障
} fsm_t;


// 初始化状态机
int fsm_init(fsm_t **f, int rfd, int wfd); // 要修改指向状态机的一级指针的值(要为状态机开辟空间，也就是让指向状态机的一级指针指向该开辟的空间)，所以要传一个二级指针

// 驱动状态机
void fsm_drive(fsm_t *f);

// 销毁状态机
int fsm_destroy(fsm_t *f);