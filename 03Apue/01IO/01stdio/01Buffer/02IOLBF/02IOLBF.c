#include <stdio.h>

int main(void){
    printf("Hello\nWorld!\n");
    /* 
    通过printf(3)往stdout输出"Hello World!\n"，stdout是行缓冲的，所以stdout每遇到换行符'\n'时会才刷新缓冲区，将内容显示到终端
    */
    while(1);
    return 0;
}