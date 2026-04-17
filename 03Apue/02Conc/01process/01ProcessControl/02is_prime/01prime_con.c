#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define MIN 100
#define MAX 300

#define NUM (MAX - MIN + 1) // 计算需要创建的子进程数量

// 判断一个数是否为质数的函数
static int is_prime(int num){
    int i = 0; // 循环变量

    sleep(1);

    // 如果num小于等于1,则不是质数
    if(num <= 1) return 0;
    // 如果num是2或3,则是质数
    if(num == 2 || num == 3) return 1;

    // 从2开始,一直循环到num的平方根,如果num能被i整除,则num不是质数
    for(i = 2; i <= num / i; i++){
        if(num % i == 0) return 0;
    }
    return 1;
}

int main(void){
    int i = 0; // 循环变量
    pid_t pid; // 存储子进程的标识

    // 循环创建MAX-MIN+1(这里是201)个子进程,每个子进程负责判断一个数是否为质数
    for(i = MIN; i <= MAX; i++){
        pid = fork(); // 创建子进程
        // 如果 创建子进程失败
        if(pid < 0){
            perror("fork()");
            exit(1);
        }
        // 子进程的操作
        if(pid == 0){
            if(is_prime(i)) printf("%d: %d Is A Prime Number\n", getpid(), i);
            exit(0);// 子进程正常终止
        }
    }

    // 父进程等待所有子进程执行结束(收尸)
    for(i = 0; i < NUM; i++) wait(NULL);

    return 0;
}