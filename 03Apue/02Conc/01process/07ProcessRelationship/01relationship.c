#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    pid_t pid;//存储子进程的标识

    printf("-------- parent --------\n");
    printf("PID:%d PPID:%d PGID:%d SID:%d\n", \
    getpid(), getppid(), getpgid(0), getsid(0));//打印父进程的信息
    printf("------------------------\n");

    pid = fork();//创建子进程
    if(pid < 0)//判断创建子进程是否失败
    {
        perror("fork()");//打印错误信息
        exit(1);//由于创建子进程失败,终止进程,并且返回状态1
    }
    if(pid == 0)//子进程的操作
    {
        printf("-------- child ---------\n");
        printf("PID:%d PPID:%d PGID:%d SID:%d\n", \
        getpid(), getppid(), getpgid(0), getsid(0));//打印子进程的信息
        printf("------------------------\n");
        exit(0);//终止子进程,并且返回状态0
    }

    wait(NULL);//父进程等待子进程结束

    return 0;
}