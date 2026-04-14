#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void){
    pid_t pid;//存储子进程的标识
    
    printf("-------- parent --------\n");
    printf("PID:%d PPID:%d PGID:%d SID:%d\n", getpid(), getppid(), getpgid(0), getsid(0));
    printf("------------------------\n");
    
    pid = fork();//创建子进程
    // 判断创建子进程是否失败
    if(pid < 0){
        perror("fork()");
        exit(1);
    }

    // 子进程的操作
    if(pid == 0){
        // 如果为子进程设置进程组失败
        if(setpgid(getpid(), getpid()) != 0){ // setpgid(getpid(), getpid()等价于setpgid(0, 0)，都是将当前进程设置为一个新的进程组的组长
            perror("setpgid()");
            exit(1); // 子进程异常退出
        }
        printf("-------- child ---------\n");
        printf("PID:%d PPID:%d PGID:%d SID:%d\n", \
        getpid(), getppid(), getpgid(0), getsid(0));
        printf("------------------------\n");
        exit(0);// 子进程正常终止，并且返回状态0
    }

    wait(NULL);//父进程等待子进程结束
    
    return 0;
}
