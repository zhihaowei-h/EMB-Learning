#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "fsm.h"

#define TTY1	"/dev/tty9"
#define TTY2	"/dev/tty10"

int main(void)
{
	fsm_t *fsm12 = NULL;//指向r9w10的有限状态机
	fsm_t *fsm21 = NULL;//指向r10w9的有限状态机
	int fd1, fd2;//存储打开文件的文件描述符

	fd1 = open(TTY1, O_RDWR);//打开/dev/tty9设备(打开时没有加入非阻塞选项)
	if(fd1 == -1)//判断打开/dev/tty9设备是否失败
	{
		perror("open()");//打印错误信息
		return -1;//由于打开/dev/tty9设备失败,结束程序,并且返回-1
	}
	write(fd1, "[****tty9****]", 14);//用来区分是/dev/tty9设备

	fd2 = open(TTY2, O_RDWR | O_NONBLOCK);//打开/dev/tty10设备(打开时加入非阻塞选项)
	if(fd2 == -1)//判断打开/dev/tty10设备是否失败
	{
		perror("open()");//打印错误信息
		close(fd1);//关闭/dev/tty9设备
		return -2;//由于打开/dev/tty10设备失败,结束程序,并且返回-2
	}
	write(fd2, "[!!!tty10!!!]", 13);//用来区分是/dev/tty10设备

	fsm_init(&fsm12, fd1, fd2);//初始化r9w10的有限状态机
	fsm_init(&fsm21, fd2, fd1);//初始化r10w9的有限状态机

	//推动有限状态机的运行(轮询: 资源消耗较大, 但实现简单)
	while(fsm12->state != STATE_T && fsm21->state != STATE_T)
	{
		fsm_drive(fsm12);//推动r9w10的有限状态机
		fsm_drive(fsm21);//推动r10w9的有限状态机
	}

	fsm_destroy(fsm12);//释放r9w10的有限状态机
	fsm_destroy(fsm21);//释放r10w9的有限状态机

	close(fd2);//关闭/dev/tty10设备文件
	close(fd1);//关闭/dev/tty9设备文件

	return 0;
}

