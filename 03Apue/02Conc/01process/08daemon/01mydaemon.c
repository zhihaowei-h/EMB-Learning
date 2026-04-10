#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 128

int main(void){
    FILE *fp = NULL; // fp指针指向打开的"/tmp/out"文件的文件流
    int ret = 0;     // 存储错误码
    time_t tm;       // 存储当前时间的时间戳
    struct tm *time_st = NULL; // 指向格式化时间所在的空间  // FIXME
    char buf[BUFSIZE] = {0};   // 用来存储格式化时间的字符串 
    
    fp = fopen("/tmp/out", "w"); // 以w的形式打开目标文件
    // 如果打开文件失败
    if(fp == NULL){
        perror("fopen()");
        ret = -1;
        goto ERR_1;
    }
    
    // 死循环
    while(1){
        // 如果获取当前时间的时间戳是否失败
        if(time(&tm) == (time_t)-1){  // FIXME
            perror("time()");
            ret = -2;
            goto ERR_2;
        }
        // 如果 把时间戳转换成格式化时间失败
        if((time_st = localtime(&tm)) == NULL){
            perror("localtime()");
            ret = -3;
            goto ERR_2;
        }

        memset(buf, 0, BUFSIZE); // 清空脏数据
        strftime(buf, BUFSIZE, "%Y年%m月%d日 %H:%M:%S\n", time_st); //把格式化时间转换成格式化时间的字符串 // FIXME
        fputs(buf, fp); // 把buf存储的字符串写入到fp指针指向的文件流中 // FIXME 
        fflush(NULL); // 刷新缓冲区  // FIXME
        sleep(1);
    }

ERR_2:
    fclose(fp);//关闭目标文件的文件流
ERR_1:
    return ret;
}