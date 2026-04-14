#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
    
jmp_buf env;
int *p = NULL;
int fd = 0;
FILE *fp = NULL;
    
void func(){
    // 1. 分配内存
    p = malloc(4);
    if (p == NULL) {
        longjmp(env, 3);  // 开辟内存失败，没有需要清理的资源
    }
    printf("内存分配成功\n");
    
    // 2. 打开 file1.txt
    fp = fopen("file1.txt", "r");
    // file1 打开失败，但内存已成功分配，需要释放内存
    if (fp == NULL) {
        longjmp(env, 2);  // 传递错误码 2
    }
    printf("文件 file1.txt 打开成功\n");
    
    // 3. 打开 file2.txt
    fd = open("file2.txt", O_RDONLY);
    // file2 打开失败，需要清理已成功的资源：p 和 fp
    if (fd < 0) {
        longjmp(env, 1);  // 传递错误码 1
    }
    printf("文件 file2.txt 打开成功\n");
}
    
int main(void){
    int ret = setjmp(env); // 首次调用 setjmp 返回 0，longjmp 跳转回来时返回非 0 值
        
    switch(ret){
        case 0:
            printf("开始分配资源...\n");
            func();
            printf("所有资源分配成功！\n");
            // 正常清理所有资源
            close(fd);
            fclose(fp);
            fp = NULL;
            free(p);
            p = NULL;
            printf("所有资源已释放完毕\n");
            break;
                
        case 1:
            printf("错误: file2.txt 打开失败\n");
            // 只需清理 fp 和 p（因为 fd 没成功）
            fclose(fp);
            fp = NULL;
            free(p);
            p = NULL;
            printf("已清理内存和 file1.txt 资源\n");
            break;
                
        case 2:
            printf("错误: file1.txt 打开失败\n");
            // 只需清理 p（因为 fp 没成功）
            free(p);
            p = NULL;
            printf("已清理内存资源\n");
            break;
                
        case 3:
            printf("错误: 内存分配失败\n");
            // 无需清理任何资源
            break;
    }
        
    return 0;
}