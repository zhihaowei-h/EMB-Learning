#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "fsm.h"

int fsm_init(fsm_t **f, int rfd, int wfd)
{
	fsm_t *me = NULL;//指向开辟的空间
	int saver = 0, savew = 0;//分别存储读文件的文件状态和写文件的文件状态

	me = malloc(sizeof(fsm_t));//开辟状态机结构的空间
	if(me == NULL)//判断开辟状态机结构的空间是否失败
		return -1;//由于开辟状态机结构的空间失败,结束函数,并且返回-1

	me->rfd = rfd;//把客户传递的读文件的文件描述符进行存储
	me->wfd = wfd;//把客户传递的写文件的文件描述符进行存储
	memset(me->buf, 0, BUFSIZE);//清空状态机结构的buf空间
	me->count = 0;//把成功读取到的字节数清0
	me->pos = 0;//把已写入的字节数清0
	me->state = STATE_R;//初始化默认状态为读的状态
	me->errmsg = NULL;//初始化出错函数的函数名指针为NULL
	
	saver = fcntl(me->rfd, F_GETFL);//获取读文件的文件状态
	fcntl(me->rfd, F_SETFL, saver | O_NONBLOCK);//在读文件原有的状态上加入非阻塞
	
	savew = fcntl(me->wfd, F_GETFL);//获取写文件的文件状态
	fcntl(me->wfd, F_SETFL, savew | O_NONBLOCK);//在写文件原有的状态上加入非阻塞

	*f = me;//把成功开辟状态机结构的空间回填

	return 0;
}

int fsm_drive(fsm_t *f)
{
	int ret = 0;//存储本次写入数据的字节数

	switch(f->state)//通过switch语句推导有限状态机
	{
		case STATE_R : 
			f->count = read(f->rfd, f->buf, BUFSIZE);//从rfd文件中读取数据
			if(f->count == -1)//判断read(2)调用是否失败
			{
				if(errno != EAGAIN)//判断是否不是假错
				{
					f->errmsg = "read()";//记录出错函数的函数名
					f->state = STATE_E;//切换到E状态
				}
			}
			else if(f->count == 0)//判断是否读到了EOF
				f->state = STATE_T;//切换到T状态
			else//读取成功
			{
				f->pos = 0;//再切换到W状态之前,先把pos清0
				f->state = STATE_W;//切换到W状态
			}
			break;
		case STATE_W : 
			ret = write(f->wfd, f->buf + f->pos, f->count);//把buf存储的数据写入到wfd中
			if(ret == -1)//判断写入数据是否失败
			{
				f->errmsg = "write()";//记录出错函数的函数名
				f->state = STATE_E;//切换到E状态
			}
			else{
				if(ret < f->count)//判断是否没有写完
				{
					f->pos += ret;//记录已经写入的字节数
					f->count -= ret;//减去已写入的字节数
				}
				else
				{
					f->state = STATE_R;//切换到R状态
				}
			}
			break;
		case STATE_E : 
			perror(f->errmsg);//打印错误信息
			f->state = STATE_T;//切换到T状态
			break;
		case STATE_T : //当前T状态什么都不做(不能直接结束进程)
			break;
		default : break;
	}

	return 0;
}

int fsm_destroy(fsm_t *f)
{
	free(f);//释放有限状态机的结构
	f = NULL;//避免出现野指针

	return 0;
}











