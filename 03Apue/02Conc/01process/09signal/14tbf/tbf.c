#include <stdio.h>
#include "tbf.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
 
#define TBFMAX 1024
 
typedef struct tbf_st{
    int token;  // 表示当前令牌桶的令牌数
    int cps;   // 表示当前令牌桶的速率
    int burst; // 当前令牌桶的容量
}tbf_t; // 令牌桶的数据结构
 
static tbf_t *tbf_libs[TBFMAX];  // 令牌桶库的定义
static int initd; // 作为是否启动令牌桶的标志(0: 空闲中 1: 工作中)
 
// SIGALRM信号的行为
static void alarm_handler(int none){
    int i = 0; // 循环变量
    alarm(1);
 
    // 遍历所有令牌桶
    for(i = 0; i < TBFMAX; i++){
        // 如果当前令牌桶正在工作中
        if(tbf_libs[i] != NULL){
            tbf_libs[i]->token += tbf_libs[i]->cps; // 给当前令牌桶积攒该令牌桶指定的令牌
 
            // 如果当前令牌桶的令牌数超过该令牌桶的上限
            if(tbf_libs[i]->token > tbf_libs[i]->burst){
                tbf_libs[i]->token = tbf_libs[i]->burst; // 把当前令牌桶中令牌数设置为该令牌桶指定的上限
            }
        }
    }
}
 
// 循环遍历令牌桶库中当前可用的最小下标的令牌桶的下标
static int get_tbf_pos(void){
    int i = 0;
 
    for(i = 0; i < TBFMAX; i++){
        // 如果 当前位置可用
        if(tbf_libs[i] == NULL){
            return i;
        }
    }
    return -1; // 没有找到合适的位置
}
 
// 启动模块
static void module_load(void){
    signal(SIGALRM, alarm_handler); //
    alarm(1);
}
 
 
// 初始化令牌桶
int tbf_init(int cps, int burst){
 
    int pos = 0; // 用来存储查询到的当前可用的下标最小的令牌桶库的下标
 
    // 如果 形参不合理
    if(cps <=  0 || burst <= 0){
        return -1;
    }
 
    // 如果 令牌桶处于空闲中
    if(!initd){
        module_load(); // 让令牌桶处于 工作中
        initd = 1;     // 切换状态为 工作中
    }
 
    pos = get_tbf_pos(); // 获取当前可用的下标最小的令牌桶库的下标
 
    // 如果 没有找到合适位置
    if(pos == -1){
        return -2;
    }
 
    tbf_libs[pos] = malloc(sizeof(TBFMAX)); // 为令牌桶开辟空间
    // 如果令牌桶空间开辟失败
    if(tbf_libs[pos] == NULL){
        return -3;
    }
 
    tbf_libs[pos]->cps = cps; // 存储客户指定的速率
    tbf_libs[pos]->burst = burst;// 存储客户指定的上限
    tbf_libs[pos]->token = 0;// 初始化令牌桶中的令牌数为0
 
    return pos; // 返回当前初始化成功的令牌桶下标
}
 
// 取走td令牌桶的n个令牌数
int tbf_fetch_token(int td, int n){
    int fetch_token = 0; // 取走的令牌数
    // 如果 参数不合理
    if(td < 0 || td >= TBFMAX || n <= 0){
        return -1;
    }
 
    // 如果 下标为td的令牌桶 不存在
    if(tbf_libs[td] == NULL){
        return -2;
    }
 
    // 等待td令牌桶积攒令牌
    while(tbf_libs[td]->token <= 0){
        pause();
    }
 
    // 如果td令牌桶的令牌充足
    if(tbf_libs[td]->token >= n){
        fetch_token = n; // 取走客户指定的令牌数
    }else{ // 如果td令牌桶的令牌不充足
        fetch_token = tbf_libs[td]->token; // 有多少取多少
    }
 
    tbf_libs[td]->token -= fetch_token; // 更新td令牌桶中的令牌数
     
    return fetch_token; // 返回成功取走的令牌数
}
 
// 销毁下标为td的令牌桶
int tbf_destroy(int td){
    // 如果 参数不合理
    if(td < 0 || td >= TBFMAX){
        return -1;
    }
 
    // 如果 要销毁的令牌库不存在
    if(tbf_libs[td] == NULL){
        return -2;
    }
 
    free(tbf_libs[td]); // 是否td令牌库
 
    tbf_libs[td] = NULL; // 避免出现野指针
 
    return 0; // 销毁成功
}