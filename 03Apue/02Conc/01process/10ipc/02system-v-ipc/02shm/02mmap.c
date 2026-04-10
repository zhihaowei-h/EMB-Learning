#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define LENGTH 1024

int main(void){
    void *ptr = NULL; // 指向映射的地址空间
    int ret = 0; // 存储错误码
    pid_t pid; // 存储子进程的标识

    ptr = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // 映射共享内存
    // 如果映射失败
    if(ptr == MAP_FAILED){
        perror("mmap()");//打印错误信息
        ret = -1;//存储-1错误码
        goto ERR_1;//由于映射共享内存失败,跳转到ERR_1的位置
    }

    //[2]创建子进程
    pid = fork();//创建子进程
    // 如果创建子进程失败
    if(pid == -1){
        perror("fork()");// 打印错误信息
        ret = -2;// 存储-2错误码
        goto ERR_2;// 由于创建子进程失败,跳转到ERR_2的位置
    }

    //[3]父子进程通过共享内存进行通信
    
    // 子进程操作
    if(pid == 0){
        memcpy(ptr, "Easthome", 8);//子进程往共享内存写入"Easthome"
        munmap(ptr, LENGTH);//子进程解除映射关系
        exit(0);//终止子进程
    }

    //父进程的操作
    wait(NULL);//等待子进程的结束
    puts(ptr);//把共享内存中的数据打印输出到标准输出

ERR_2 :
    munmap(ptr, LENGTH);//解除映射关系
ERR_1 :
    return ret;
}
