#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
  
jmp_buf env;
int *p = NULL;
int fd = 0;
FILE *fp = NULL;
  
void func(){
    p = malloc(4); // 在堆区开辟一个4字节的内存空间
    if (p == NULL) longjmp(env, 3);
    printf("内存分配成功\n");
  
    fp = fopen("file1.txt", "r");
    if (fp == NULL) longjmp(env, 2);
    printf("文件 file1.txt 打开成功\n");
  
    fd = open("file2.txt", O_RDONLY);
    if (fd < 0) longjmp(env, 1);
    printf("文件 file2.txt 打开成功\n");
}
  
int main(void){
    int ret = setjmp(env);
      
    switch(ret){
        case 0:
            printf("开始分配资源...\n");
            func();
            printf("所有资源分配成功！\n");
            // 开辟空间成功，打开file1.txt文件成功，打开file2.txt文件成功，需要释放全部资源
            close(fd);
        case 1: // 开辟空间成功，打开file1.txt文件成功，打开file2.txt文件失败，所以只需要释放开辟的空间和打开的文件
            fclose(fp); fp = NULL;
        case 2: // 开辟空间成功，但是打开file1.txt文件失败，还没有打开file2.txt文件，所以只需要释放开辟的空间
            free(p); p = NULL;
        case 3:
            printf("资源已清理\n");
            break;
    }
      
    return 0;
}