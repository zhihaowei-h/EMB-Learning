// 需求: [1]号子进程执行ps-ajx [2]号子进程执行grep "systemd"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFSIZE 128

int main(int argc, char *argv[]){
    
    int pfd[2] = {0}; // 存储管道的两个文件描述符，fd[0]用于读，fd[1]用于写
    int ret = 0;      // 存储函数调用的返回值
    pid_t pid = 0;    // 存储进程ID
    char buf[BUFSIZE] = {0}; // 存储从管道中读取的数据

    // 如果 父进程创建管道失败
    if(pipe(pfd) == -1){
        perror("pipe");
        ret = -1;
        goto ERR_1;
    }

    pid = fork(); // 创建子进程
    
    // 如果创建子进程失败
    if(pid == -1){
        perror("fork");
        ret = -2;
        goto ERR_2;
    }

    // [1]号子进程操作
    if(pid == 0){ 
        close(pfd[0]); // 关闭读端文件描述符
        dup2(pfd[1], 1); // 将管道的写端重定向到标准输出
        execl("/bin/ps", "ps", "-ajx", NULL); // 执行ps-ajx命令，输出结果到管道中
        close(pfd[1]); // 关闭写端文件描述符
        exit(0);   // 退出子进程
    }

    pid = fork(); // 创建第二个子进程

    // 如果创建第二个子进程失败
    if(pid == -1){
        perror("fork");
        ret = -3;
        goto ERR_3;
    }

    // [2]号子进程操作
    if(pid == 0){ 
        close(pfd[1]); // 关闭写端文件描述符
        dup2(pfd[0], 0); // 将管道的读端重定向到标准输入
        execl("/bin/grep", "grep", "systemd", NULL); // 执行grep命令，从管道中读取数据并过滤出包含"systemd"的行
        close(pfd[0]); // 关闭读端文件描述符
        exit(0);   // 退出子进程
    }

    close(pfd[0]); // 父进程关闭读端文件描述符
    close(pfd[1]); // 父进程关闭写端文件描述符
    wait(NULL); // 等待子进程结束
    wait(NULL); // 等待第二个子进程结束

    return 0; // 正常退出

ERR_3:
    close(pfd[0]); // 关闭读端文件描述符
    close(pfd[1]); // 关闭写端文件描述符
ERR_2:
ERR_1:
    return ret; // 返回错误码
}