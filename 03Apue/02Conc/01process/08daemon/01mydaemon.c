// 目标: 让调用进程每隔1秒钟就把当前的时间写入到"/tmp/out"文件中
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 128

int main(void){
    FILE *fp = NULL;
    int ret = 0;     // 存储错误码
    time_t tm;       // 用于存储当前时间的时间戳
    struct tm *time_st = NULL; // 指向struct tm时间结构体
    char buf[BUFSIZE] = {0};   // 用于存储 格式化时间 的字符串 
    
    fp = fopen("/tmp/out", "w");
    // 如果打开文件失败
    if(fp == NULL){
        perror("fopen()");
        ret = -1;
        goto ERR_1;
    }
    
    // 死循环
    while(1){
        // 如果获取当前时间的时间戳是否失败
        if(time(&tm) == (time_t)-1){
            perror("time()");
            ret = -2;
            goto ERR_2;
        }
        // 如果 把时间戳转换成格式化时间结构体失败
        if((time_st = localtime(&tm)) == NULL){
            perror("localtime()");
            ret = -3;
            goto ERR_2;
        }

        memset(buf, 0, BUFSIZE); // 清空脏数据
        strftime(buf, BUFSIZE, "%Y年%m月%d日 %H:%M:%S\n", time_st); // 把 格式化时间结构体 转换成 格式化时间字符串，存储在buf数组中
        fputs(buf, fp); // 把buf存储的字符串写入到fp指针指向的FILE结构体所管理的缓冲区中
        fflush(NULL); // 刷新缓冲区，参数为NULL，表示刷新所有的输出流的缓冲区
        sleep(1);
    }

ERR_2:
    fclose(fp);
ERR_1:
    return ret;
}