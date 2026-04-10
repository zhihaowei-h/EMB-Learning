#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    printf("PID : %d\n", getpid());//获取当前调用进程的PID
    printf("PPID : %d\n", getppid());//获取当前调用进程的父进程的PID
    
    sleep(1000);//睡1000s
    
    return 0;
}