#include <stdio.h>
#include <unistd.h>

int main(void){
    printf("Begin...\n");// 打印开始
 
    execl("/usr/bin/ls", "ls", "-l", NULL); // 使用execl函数替换当前进程为ls命令的进程，并传递参数-l
    
    printf("End...\n");// 打印结束
    
    return 0;
}