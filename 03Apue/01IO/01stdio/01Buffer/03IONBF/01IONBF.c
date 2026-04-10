#include <stdio.h>
#include <unistd.h>
  
int main(void){
    // stdout 的缓冲区是是行缓冲区
    printf("stdout: 这是行缓冲的消息");
    // stderr 无缓冲区
    fprintf(stderr, "stderr: 这是无缓冲的消息");
    // 进入死循环
    while(1);
  
    return 0;
}