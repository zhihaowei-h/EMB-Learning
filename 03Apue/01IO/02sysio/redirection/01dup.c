#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
  
int main() {
    // 创建或打开一个文件
    int fd1 = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    printf("原始文件描述符: %d\n", fd1);  // 通常是 3
      
    // 复制文件描述符
    int fd2 = dup(fd1); // 将fd1复制到fd2(最小的可用是 4)
    printf("复制后的文件描述符: %d\n", fd2);
      
    // 通过 fd1文件描述符 写入
    write(fd1, "Hello from fd1\n", 15);
      
    // 通过 fd2文件描述符 写入(同一个文件表项)
    write(fd2, "Hello from fd2\n", 15);
      
    // 关闭一个不影响另一个
    close(fd1); // 让文件标识符fd1的文件指针指向NULL
      
    // fd2 仍然有效
    write(fd2, "fd1 closed but fd2 still works.\n", 32);
      
    close(fd2); // 让文件标识符fd2的文件指针指向NULL
    return 0;
}