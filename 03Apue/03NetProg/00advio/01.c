#include <unistd.h>
#include <stdio.h>

int main() {
    char buf[1024];
    printf("等待输入(阻塞模式)...\n");
    
    // 如果你在终端不输入内容，程序会一直停在这里
    // 只有当你按下回车，或者按 Ctrl+D (EOF) 时才会返回
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf)); 
    
    if (n > 0) {
        printf("读到了 %ld 字节数据\n", n);
    } else if (n == 0) {
        printf("读到了 EOF，用户关闭了输入\n");
    }
    return 0;
}