#ifndef __POOL_H
#define __POOL_H

#include <pthread.h>
#include "queue.h"

#define MAXJOB		10//最大任务数量(测试版)
#define MIN_FREE_NR	3//最小空闲线程的个数
#define MAX_FREE_NR	5//最大空闲线程的个数
#define STEP		2//增量(增加或减少线程的数量)

//[1]定义线程池结构体
typedef struct
{
	pthread_t *workers;//工作线程结构的起始地址(动态开辟)
	pthread_t admin_tid;//管理者线程标识
	queue_t *task_queue;//任务队列
	//线程池结构
	int max_threads;//最多线程容量
	int min_free_threads;//最少空闲线程个数
	int max_free_threads;//最多空闲线程个数
	int busy_threads;//busy线程数
	int live_threads;//live线程数
	int exit_threads;//管理者线程当需要线程终止的时候,进行记录
	int shutdown;//标记是否关闭线程池(0正常工作 1线程池关闭了)
	pthread_mutex_t mut_pool;//整个线程池的互斥量
	pthread_mutex_t mut_busy;//busy单独加一个互斥量
	pthread_cond_t queue_not_empty;//如果任务队列不为空,通知取任务
	pthread_cond_t queue_not_full;//如果任务队列不为满,通知添加新任务
}pool_t;

//任务结构
typedef struct
{
	void *(*job)(void *s);//存储执行任务的地址
	void *arg;//执行任务的参数
}task_t;

//[2]函数的接口
/*
功能 : 初始化线程池
参数 : mypool 把创建成功的线程池地址进行回填
		capacity 容量(最多线程容量)
返回值 : 成功返回0;失败返回<0
*/
extern int pool_init(pool_t **mypool, int capacity);

/*
功能 : 添加任务
参数 : mypool 要操作的线程池
		t 要添加的任务
返回值 : 成功返回0;失败返回<0
*/
extern int pool_add_task(pool_t *mypool, const task_t *t);

/*
功能 : 销毁线程池
参数 : mypool 要销毁的线程池
返回值 : 空
*/
extern void pool_destroy(pool_t *mypool);

#endif







