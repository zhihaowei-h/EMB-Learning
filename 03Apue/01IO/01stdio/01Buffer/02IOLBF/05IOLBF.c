#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void){
    printf("dmosiap0md0-samodmsi0paom0diasmpaHello World!");
    
    _exit(0); // 使用 _exit(2) 直接退出程序，不会刷新stdout的缓冲区，所以 "Hello World!" 不会被写入终端
}