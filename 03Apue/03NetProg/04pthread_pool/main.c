#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pool.h"

static void *task1(void *arg)
{
	int i = 0;
	for(i = 0; i < 10; i++)//模拟线程要做的任务
		sleep(1);
	return NULL;
}

int main(void)
{
	pool_t *p = NULL;//p指针指向线程池
	int i = 0;//循环变量
	task_t t;//定义任务结构的变量

	pool_init(&p, MAXJOB);//初始化线程池
	
	sleep(5);//睡5s

	for(i = 0; i < 15; i++)
	{
		t.job = task1;
		t.arg = NULL;
		pool_add_task(p, &t);//添加任务
	}

	while(1);

	return 0;
}





