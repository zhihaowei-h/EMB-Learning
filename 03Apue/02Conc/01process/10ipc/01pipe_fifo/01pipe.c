// 父子进程通过文件进行通信，父进程等待子进程写入数据后再读取数据并打印出来。
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFSIZE 128


int main(int argc, char *argv[]){
    int fd = 0;    // 存储打开文件的文件描述符
    int ret = 0;   // 存储函数调用的返回值
    pid_t pid = 0; // 存储进程的ID
    char buf[BUFSIZE] = {0}; // 存储从文件中读取的数据

    fd = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); // 如果文件已经存在，则以只读方式打开文件并且清空文件内容，并获取文件描述符; 如果文件不存在则创建一个文件(权限为0666)

    // 如果open(2)文件失败
    if(fd == -1){
        perror("open");
        ret = -1;
        goto ERR_1;
    }

    pid = fork(); // 创建子进程
    // 如果fork(2)创建子进程失败
    if(pid == -1){
        perror("fork");
        ret = -2;
        goto ERR_2;
    }

    // 子进程操作
    if(pid == 0){ 
        write(fd, "Hello World!\n", 12); // 往文件描述符fd所指的文件中写入12个字节的数据
        close(fd); // 关闭文件描述符
        exit(0);   // 退出子进程
    }

    // 父进程操作
    wait(NULL); // 等待子进程结束
    lseek(fd, 0, SEEK_SET); // 将fd文件描述符所指文件的偏移量设置为文件开头
    read(fd, buf, BUFSIZE); // 从fd所指文件中读取12个字节的数据到buf中
    close(fd);
    printf("Parent: %s\n", buf); // 打印从文件中读取的数据

    return 0; // 父进程正常退出

ERR_2:
    close(fd); // 关闭文件描述符
ERR_1:
    return ret;
}