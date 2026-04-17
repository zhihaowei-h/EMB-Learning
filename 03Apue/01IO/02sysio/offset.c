// 当前文件偏移量/文件句柄的位置
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
  
int main() {
    int fd = open("offset.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
      
    // 初始偏移量是 0
    printf("初始偏移量: %ld\n", lseek(fd, 0, SEEK_CUR));
      
    // 写入 "Hello" (5字节) → 偏移量变成 5
    write(fd, "Hello", 5);
    printf("写入后偏移量: %ld\n", lseek(fd, 0, SEEK_CUR));
      
    // 再写入 " World" (6字节) → 偏移量变成 11
    write(fd, " World", 6);
    printf("再次写入后偏移量: %ld\n", lseek(fd, 0, SEEK_CUR));
      
    // 手动移动偏移量
    lseek(fd, 0, SEEK_SET);  // 回到开头
    printf("回到开头: %ld\n", lseek(fd, 0, SEEK_CUR));
      
    // 读取 5 字节 → 偏移量变成 5
    char buf[10];
    read(fd, buf, 5);
    printf("读取后偏移量: %ld\n", lseek(fd, 0, SEEK_CUR));
      
    // 不同进程/描述符共享同一个偏移量？
    int fd2 = dup(fd);  // 复制文件描述符
    printf("fd2 的偏移量: %ld\n", lseek(fd2, 0, SEEK_CUR));
      
    // fd 移动偏移量
    lseek(fd, 0, SEEK_END);
    printf("fd 移到末尾: %ld\n", lseek(fd, 0, SEEK_CUR));
    printf("fd2 的偏移量: %ld\n", lseek(fd2, 0, SEEK_CUR)); // 也跟着变
      
    close(fd);
    close(fd2);
    return 0;
}