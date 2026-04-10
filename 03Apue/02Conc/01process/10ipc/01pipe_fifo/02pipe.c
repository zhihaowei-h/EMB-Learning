// 使用匿名管道实现父子进程间通信(父进程读，子进程写)
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define BUFSIZE 128

int main(void){
    int pipe_id[2] = {0};
    char buf[BUFSIZ] = {0}; // 存储父进程读取到的数据
    int ret = 0; // 错误码
    pid_t pid; // 子进程的id

    // 如果创建管道失败
    if(pipe(pipe_id) == -1){
        perror("pipe()");
        ret = -1;
        goto ERR_1;
    } 

    // 创建子进程
    pid = fork();
    // 如果 创建子进程失败
    if(pid == -1){
        perror("fork()");
        ret = -2;
        goto ERR_2;
    }

    // 子进程操作
    if(pid == 0){
        close(pipe_id[0]); // 关闭子进程的对于管道读端的文件描述符
        write(pipe_id[1], "hello", 5); // 从管道的写端往管道写入5字节的数据
        close(pipe_id[1]); // 关闭子进程的对于管道写端的文件描述符
        exit(0); // 子进程结束
    }

    // 父进程操作
    wait(NULL);
    close(pipe_id[1]); // 关闭父进程的对于管道写端的文件描述符
    read(pipe_id[0], buf, 5); // 从管道的读端读取5个字节的数据
    printf("父进程从管道中读取到的数据: %s\n", buf);
    close(pipe_id[0]); // 关闭父进程的对于管道写端的文件描述符

    return 0;

ERR_2:
    close(pipe_id[0]); // 关闭读端文件描述符
    close(pipe_id[1]); // 关闭写端文件描述符
ERR_1:
    return ret;

}