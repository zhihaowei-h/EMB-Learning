#pragma once

typedef void (*HANDLER)(void *); // 回调函数的定义



/*
功能: 初始化闹钟
参数: 
· seconds: 要定时的秒数
· handler: 要做的事情
· arg: 要做的事情的参数
返回值: 成功则返回闹钟库的下标(>=0); 失败则返回负数(<0)
*/
int anytimer_init(int seconds, HANDLER handler, void *arg);



/*
功能: 销毁指定下标的闹钟
参数: 
· timer_id: 要销毁的闹钟的标识
返回值: 成功则返回0；失败则返回负数
*/
int anytimer_destory(int timer_id);