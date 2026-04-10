#pragma once

#define BUFSIZE 1024 // 定义BUFFER空间大小

// 定义状态
enum {
    STATE_R = 0, // 读状态
    STATE_W,     // 写状态
    STATE_E,     // 异常状态
    STATE_T      // 终止状态
};

// 有限状态机结构体
typedef struct{
    int rfd;           // 读文件描述符
    int wfd;           // 写文件描述符
    char buf[BUFSIZE]; // BUFFER空间
    int count;         // 已读取的字节个数
    int pos;           // 已写入的字节个数
    int state;         // 有限状态机当前的状态
    char *errmsg;      // 出错函数的函数名
} fsm_t;

// 定义接口

/**
 * @brief 
 * 初始化有限状态机
 * @param
 * f: 待初始化的有限状态机
 * rfd: 读文件描述符
 * wfd: 写文件描述符
 * @return
 * 成功返回0，失败返回<0
 * 
 * 要做两件事: 
 * [1]给有限状态机开辟空间
 * [2]初始化有限状态机的成员变量
 * 
 * 注意: 如何保证客户传递过来的rfd、wfd是非阻塞的: 可以使用fcntl(2)先获取原有的文件状态标志，然后在原有的基础上添加O_NONBLOCK标志，最后再把新的文件状态标志设置回去。
 */
extern int fsm_init(fsm_t **f, int rfd, int wfd);

/**
 * @brief 
 * 驱动有限状态机
 * @param
 * f: 待驱动的有限状态机
 * 
 */
extern void fsm_drive(fsm_t *f);

/**
 * @brief 
 * 功能: 销毁有限状态机
 * @param
 * f: 待销毁的有限状态机
 * @return
 * 成功返回0，失败返回<0
 * 
 * 判断有限状态机的状态
 * [R] 把rfd文件的数据读到有限状态机的BUFFER空间中，更新count成员变量，切换状态为W
 *      > 0 成功读取到数据，更新count成员变量，切换状态为W
 *      = 0 EOF 读到文件末尾，切换状态为[W]
 *      < 0 读错误，判断是不是假错误，如果是EAGAIN错误，重读，推到状态为R，如果不是EAGAIN错误，更新errmsg成员变量指向出错函数的函数名，切换状态为E
 * [W] 把有限状态机的BUFFER空间中的数据写到wfd文件中(判断是否要写入的数据都写入了，如果没有写完，就续写)
 * [E] 输出errmsg成员变量指向的出错函数的函数名，切换状态为T
 * [T] 销毁有限状态机
 */
extern void fsm_drive(fsm_t *f);

/**
 * @brief 
 * 销毁有限状态机
 * @param
 * f: 待销毁的有限状态机
 * @return
 * 成功返回0，失败返回<0
 */
extern int fsm_destroy(fsm_t *f);