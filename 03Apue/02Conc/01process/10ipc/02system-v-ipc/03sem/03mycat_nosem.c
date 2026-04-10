// 创建4个子进程,实现mycat功能，不用信号量数组
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
 
 
int main(void){
    int i = 0; // 循环变量
    int j = 0; // 循环变量
    int count = 0; // 质数计数器
    int pid = 0; // 存储子进程的PID
 
    // 循环100-300中的每个数，创建子进程来判断该数是否为质数
    for(i = 100; i <= 300; i++){
        // 创建子进程
        pid = fork();
        // 如果创建子进程失败
        if(pid == -1){
            perror("fork()");
            exit(-1);
        }
        // 子进程操作
        if(pid == 0){
            count = 0;
            // 判断i是否为质数
            for(j = 2; j < i; j++){
                if(i % j == 0){
                    count++;
                    break;
                }
            }
            // 如果count为0，说明i是质数，打印i
            if(count == 0){
                printf("%d\n", i);
            }
            exit(0); // 子进程退出
        }
    }
 
    // 父进程等待所有子进程结束
    for(i = 100; i <= 300; i++){
        wait(NULL);
    }
    return 0;
}