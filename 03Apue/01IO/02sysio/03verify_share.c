#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd1 = open("test.txt", O_RDONLY); // fd1 --> 文件表项(文件状态标志: O_RDONLY) --> test.txt的inode表项
    int fd2 = dup(fd1); // 复制描述符 // dup 复制的是进程表里的“指针”，而不是内核里的“文件表项”。所以 fd1 改了表项里的状态，fd2 也会感应到。
    /* 在int fd2 = dup(fd1)后: 
    fd1 --> 文件表项(文件状态标志: O_RDONLY) --> test.txt的inode表项
                ↑
               fd2
    */
    printf("Original fd1 flags modified...\n");

    // 1. 通过 fd1 修改其所指文件表项中的文件标志位
    int flags = fcntl(fd1, F_GETFL);
    fcntl(fd1, F_SETFL, flags | O_NONBLOCK); // 给 fd1 所指文件表项添加 O_NONBLOCK 标志

    // 2. 检查 fd2 的状态是否也随之改变
    int flags2 = fcntl(fd2, F_GETFL);
    if (flags2 & O_NONBLOCK) {
        printf("Verification Success: fd2 is now Non-blocking too!\n");
        printf("Reason: Both fds share the SAME File Table Entry in kernel.\n");
    }

    close(fd1);
    close(fd2);
    return 0;
}