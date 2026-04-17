#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("test.txt", O_RDWR | O_APPEND | O_CREAT, 0644); // fd --> 文件表项(文件状态标志: O_RDWR | O_APPEND | O_CREAT) --> test.txt的inode表项
    if (fd < 0) return 1;

    // 获取文件状态标志
    int flags = fcntl(fd, F_GETFL); // 获取fd对应的文件表项中的文件状态标志
    // 如果 获取文件状态标志失败
    if (flags == -1){
        close(fd);
        return -1;
    }

    // 1. 处理访问模式（必须先屏蔽其他位）
    int mode = flags & O_ACCMODE;
    if (mode == O_RDONLY)      printf("Access: Read Only\n");
    else if (mode == O_WRONLY) printf("Access: Write Only\n");
    else if (mode == O_RDWR)   printf("Access: Read Write\n");

    // 2. 处理布尔型标志
    if (flags & O_APPEND)   printf("Flags: Append mode is ON\n");
    if (flags & O_NONBLOCK) printf("Flags: Non-blocking is ON\n");

    close(fd);
    return 0;
}