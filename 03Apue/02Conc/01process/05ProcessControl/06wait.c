#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;//存储子进程的标识
    int status;//存储子进程退出时的状态
    pid_t t_pid;//存储被终止的子进程的标识

    pid = fork();//创建子进程
    if(pid < 0)//判断创建子进程是否失败
    {
        perror("fork()");//打印错误信息
        exit(1);//由于创建子进程失败,终止进程,并且返回状态1
    }

    if(pid == 0)//子进程的分支,先进行打印,然后退出
    {
        printf("======== 子进程(PID:%d)开始执行 ========\n", getpid());
        sleep(10);//模拟子进程正在做的任务
        printf("==== 子进程(PID:%d)执行完毕,准备退出 ===\n", getpid());
        exit(10);//子进程退出,状态码为10
    }
    else//父进程的分支,等待子进程结束后再打印
    {
        printf("父进程(PID:%d):等待子进程(PID:%d)执行完毕...\n", getpid(), pid);
        t_pid = wait(&status);//等待子进程执行结束
        if(t_pid == -1)//判断收尸是否失败
        {
            perror("wait()");//打印错误信息
            exit(2);//由于收尸失败,终止进程,并且返回状态2
        }
        if(WIFEXITED(status))//判断子进程是否正常终止
        {
            printf("父进程:子进程(PID:%d)已正常终止,退出状态码为:%d\n", t_pid, WEXITSTATUS(status));
        }
        printf("==== 父进程(PID:%d)开始执行后续操作 ====\n", getpid());
    }

    return 0;
}