#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(void)
{
	struct rlimit limit;//用来存储获取到的进程资源限制的信息

	if(getrlimit(RLIMIT_STACK, &limit) != 0)//判断获取进程资源限制的信息是否失败
	{
		perror("getrlimit()");//打印错误信息
		return -1;//由于获取进程资源限制的信息失败,结束程序,并且返回-1
	}

	printf("Stack Limit : %ldKB\n", limit.rlim_cur >> 10);

	return 0;
}