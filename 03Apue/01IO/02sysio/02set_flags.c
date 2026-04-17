#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("test.txt", O_RDWR); // fd --> 文件表项(文件状态标志: O_RDWR) --> test.txt的inode表项
    
    int flags = fcntl(fd, F_GETFL); // 获取fd对应的文件表项中的文件状态标志
    // 如果 获取文件状态标志失败
    if (flags == -1){
        close(fd);
        return -1;
    }

    // 动态添加非阻塞标志O_NONBLOCK
    flags |= O_NONBLOCK;
    
    // 写回文件表项
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL");
        close(fd);
        return -2;
    }
    
    printf("Successfully added O_NONBLOCK to file table entry.\n");
    
    close(fd);
    return 0;
}