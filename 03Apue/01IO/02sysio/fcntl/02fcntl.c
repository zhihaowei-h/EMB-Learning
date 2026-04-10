// 演示fcntl(2)的功能2: 获得/设置文件描述符标志(cmd = F_GETFD或F_SETFD) 
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fdflags = 0; // 文件描述符标志
    int fd = open("test.txt", O_RDONLY | O_CREAT, 0644);

    if (fd == -1) { 
        perror("open"); 
        exit(1); 
    }

    // 1. 获取文件描述符fd文件描述符标志(因为文件描述符标志只有FD_CLOEXEC一个，如果该fd有这个标志，那么flags的值就是FD_CLOEXEC，否则就是0；如果获取失败了，返回-1)
    fdflags = fcntl(fd, F_GETFD);
    if (fdflags == -1) { 
        perror("fcntl F_GETFD"); 
        exit(2); 
    }

    // 2. 检查是否设置了 FD_CLOEXEC
    if (fdflags & FD_CLOEXEC) {
        printf("fd 已设置 FD_CLOEXEC\n");
    } else {
        printf("fd 未设置 FD_CLOEXEC，现在开始设置...\n");
    }

    // 3. 在原有标志基础上，追加 FD_CLOEXEC 并设置回去
    fdflags |= FD_CLOEXEC;
    // 如果将fd的文件描述符标志设置为fdflags失败
    if (fcntl(fd, F_SETFD, fdflags) == -1) {
        perror("fcntl F_SETFD"); 
        exit(3);
    }

    printf("设置成功！<如果现在调用 exec()，FD %d 会被内核自动关闭>\n", fd);

    // 再把 FD_CLOEXEC 标志去掉，看看效果
    fdflags &= ~FD_CLOEXEC;
    if (fcntl(fd, F_SETFD, fdflags) == -1) { 
        perror("fcntl F_SETFD"); 
        exit(4);
    }

    close(fd);
    return 0;
}