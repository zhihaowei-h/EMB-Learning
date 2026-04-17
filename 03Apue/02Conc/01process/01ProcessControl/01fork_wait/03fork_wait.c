#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void){
    pid_t pid; // 存储子进程的标识
    int status; // 存储子进程退出时的状态
    pid_t t_pid; // 存储子进程的标识

    pid = fork(); // 创建子进程
    // 判断创建子进程是否失败
    if(pid < 0){
        perror("fork()");
        exit(1);
    }

    // 子进程操作
    if(pid == 0){
        printf("======== 子进程(PID:%d)开始执行 ========\n", getpid());
        sleep(10);//模拟子进程正在做的任务
        printf("==== 子进程(PID:%d)执行完毕,准备退出 ===\n", getpid());
        exit(10); // 子进程退出,状态码为10
    }else if(pid > 0){ // 父进程操作
        printf("父进程(PID:%d):等待子进程(PID:%d)执行完毕...\n", getpid(), pid);
        t_pid = wait(&status); // 等待子进程执行结束，并获取子进程的退出状态
        // 判断收尸是否失败
        if(t_pid == -1){
            perror("wait()");
            exit(2);
        }
        // 如果子进程正常终止
        if(WIFEXITED(status)){
            printf("父进程: 子进程(PID:%d)已正常终止,退出状态码为%d\n", t_pid, WEXITSTATUS(status));
        }
        printf("==== 父进程(PID:%d)开始执行后续操作 ====\n", getpid());
    }

    return 0;
}