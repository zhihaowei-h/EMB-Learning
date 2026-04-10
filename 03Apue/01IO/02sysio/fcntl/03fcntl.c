// 演示fcntl(2)的功能3: 获得/设置文件状态标志(cmd = F_GETFL或F_SETFL)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    // 初始打开文件时没有给该文件标识符添加 O_APPEND 这个文件状态标识
    int fd = open("test.txt", O_WRONLY | O_CREAT, 0644);
    // 如果打开文件失败
    if (fd == -1) { 
        perror("open"); 
        exit(1); 
    }

    // 1. 获取文件描述符fd的文件状态标志
    int flags = fcntl(fd, F_GETFL);
    // 如果获取文件描述符的文件状态标志失败
    if (flags == -1) { 
        perror("fcntl F_GETFL"); 
        exit(2); 
    }

    // 2. 动态加上追加写入标志 O_APPEND
    flags |= O_APPEND;  // 此时flags中就包含了 O_RDONLY, O_WRONLY, O_APPEND 3种标志了
    // 如果将fd的文件状态标志设置为flags失败
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL"); 
        exit(3);
    }

    printf("成功为 FD %d 添加了 O_APPEND (追加模式) 标志\n", fd);
    write(fd, "Appended text\n", 14);

    close(fd);
    return 0;
}