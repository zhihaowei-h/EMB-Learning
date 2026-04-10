// 创建4个子进程,实现查询100-300中的质数，不用信号量数组
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int main(void) {
    int i, j, n;
    pid_t pid;

    // 循环创建 4 个子进程(编号分别为 0, 1, 2, 3)
    for (n = 0; n < 4; n++) {
        pid = fork();
        // 如果 创建子进程失败
        if (pid == -1) {
            perror("fork()");
            exit(1);
        }
        if(pid == 0){
            break;
        }
    }

    // 计算并打印质数(编号为0、1、2、3的子进程分别计算100-300之间的质数，父进程也会计算100-300之间的质数)
    for(i = 100 + n; i <= 300; i += 4) {
        int is_prime = 1;
        // 判断 i 是否为质数
        for (j = 2; j <= i / 2; j++) {
            if (i % j == 0) {
                is_prime = 0;
                break;
            }
        }
        // 如果 i 是质数，则打印出来
        if (is_prime) {
            printf("[%d]%d\n", n, i);
        }
    }

    // 子进程退出
    switch(n){
        case 0:
        case 1:
        case 2:
        case 3: exit(0);
    }

    // 父进程等待所有子进程结束
    for(i = 0; i < 4; i++) {
        wait(NULL);
    }
    return 0;
}