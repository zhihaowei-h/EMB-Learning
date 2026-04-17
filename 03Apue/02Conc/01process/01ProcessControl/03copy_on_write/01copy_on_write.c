#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void){
    int i = 10010; // 存储特定值
    pid_t pid; // 存储子进程的标识

    pid = fork(); // 创建子进程
    // 如果 创建子进程失败
    if(pid < 0){
        perror("fork()");
        exit(1);
    }

    // 子进程操作
    if(pid == 0){
        printf("child.i = %p i = %d\n", &i, i);
        i = 10086; // 此时就会触发写时复制机制，子进程会创建一个新的内存页来存储i的值，父进程的i值不受影响
        printf("child.i = %p i = %d\n", &i, i);
        exit(0); // 子进程正常终止
    }else if(pid > 0){ // 父进程的操作
        wait(NULL); // 父进程等待子进程结束
        printf("parent.i = %p i = %d\n", &i, i);
        i = 12306;
        printf("parent.i = %p i = %d\n", &i, i);
    }

    return 0;
}