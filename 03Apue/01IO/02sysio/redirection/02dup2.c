#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
  
int main() {
    // 打开一个文件
    int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    printf("原始文件描述符: %d\n", fd);  // 通常是 3
      
    // 将文件描述符fd 复制到文件描述符 5(另一个说法: 让文件描述符5作为文件描述符fd的副本)
    int newfd = dup2(fd, 5);
    printf("dup2(fd, 5) 返回: %d\n", newfd);  // 5
      
    // 通过两个描述符写入(同一个管子上有俩编号，叫谁都是这一个管子)
    write(fd, "通过 fd 写入\n", 17);
    write(5, "通过 fd=5 写入\n", 19);
      
    close(fd); // 把编号fd的管子给我删了，这个管子没了，那这个管子上所有的编号都应该回收
    // 注意：close(fd) 后，fd=5 也被关闭（因为共享同一文件表项）
      
    return 0;
}