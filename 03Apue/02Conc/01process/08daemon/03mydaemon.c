// 使用daemon(2)创建 一个守护进程，实现01mydaemon.c的功能
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 128

int main(void){
    FILE *fp = NULL;//fp指针指向打开的"/tmp/out"文件的文件流
    int ret = 0;//ret存储错误码
    time_t tm;//存储当前时间的时间戳
    struct tm *time_st = NULL;//time_st指针指向格式化时间的空间
    char buf[BUFSIZE] = {0};//buf数组用来存储格式化时间的字符串

    // 如果 创建守护进程失败
    if(daemon(0, 0) == -1){
        perror("daemon()");
        goto ERR_1;
    }

    // 注意: 下面都是守护进程的操作(调用进程在daemon()函数中已经被杀死了,所以下面的代码都是在守护进程中执行的)
    fp = fopen("/tmp/out", "w");// 以w的形式打开目标文件
    // 如果打开文件失败
    if(fp == NULL){
        perror("fopen()");
        ret = -1;
        goto ERR_1;
    }

    //死循环
    while(1){
        // 如果获取当前时间的时间戳失败
        if(time(&tm) == (time_t)-1){
            perror("time()");
            ret = -2;
            goto ERR_2;
        }
        // 如果把tm时间戳转换成格式化时间结构体失败
        if((time_st = localtime(&tm)) == NULL){
            perror("localtime()");
            ret = -3;
            goto ERR_2;
        }
        //memset();//清空脏数据
        strftime(buf, BUFSIZE, "%Y年%m月%d日 %H:%M:%S\n", time_st); //把 格式化时间 转换成 格式化时间的字符串
        fputs(buf, fp); // 把buf存储的字符串写入到fp指针指向的文件流中
        fflush(NULL); // 刷新输出缓冲区
        sleep(1);
    }

ERR_2:
    fclose(fp);//关闭目标文件的文件流
ERR_1:
    return ret;
}