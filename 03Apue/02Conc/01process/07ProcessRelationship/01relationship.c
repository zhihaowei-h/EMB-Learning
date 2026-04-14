#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    pid_t pid;//存储子进程的标识

    printf("-------- parent --------\n");
    printf("PID:%d PPID:%d PGID:%d SID:%d\n", getpid(), getppid(), getpgid(0), getsid(0)); // 打印父进程的信息
    printf("------------------------\n");

    pid = fork();//创建子进程
    // 如果创建子进程失败
    if(pid < 0){
        perror("fork()");
        exit(1); // 父进程异常退出
    }

    // 子进程操作
    if(pid == 0){
        printf("-------- child ---------\n");
        printf("PID:%d PPID:%d PGID:%d SID:%d\n", \
        getpid(), getppid(), getpgid(0), getsid(0)); // 打印子进程的信息
        printf("------------------------\n");
        exit(0); // 子进程正常退出
    }

    wait(NULL); // 父进程等待子进程结束

    return 0;
}