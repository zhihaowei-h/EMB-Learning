// 演示fcntl(2)的功能1: 复制一个现有的文件描述符(cmd = F_DUPFD)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int new_fd = 0; // 新的文件描述符
    int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    // 如果打开文件失败
    if (fd == -1) { 
        perror("open"); 
        exit(1); // 由于打开文件失败，退出程序，并返回错误码1 
    }

    // 复制fd到新的文件描述符，並且新的文件描述符必须 >= 10
    new_fd = fcntl(fd, F_DUPFD, 10);
    if (new_fd == -1) { 
        perror("fcntl F_DUPFD"); 
        exit(2); // 由于复制文件描述符失败，退出程序，并返回错误码2 
    }

    printf("旧的 FD: %d\n", fd);
    printf("新的 FD: %d (如我们所愿，它 >= 10)\n", new_fd);

    write(new_fd, "Hello via new_fd\n", 17); // 通过新的文件描述符写入数据到文件

    close(fd);
    close(new_fd);
    return 0;
}