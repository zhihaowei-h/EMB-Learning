// 使用setsid(2)通过守护进程的标准步骤封装一个 mydaemon 函数，实现01mydaemon.c的功能
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFSIZE 128

// 封装一个 mydaemon 函数，完成守护进程的标准步骤
static int mydaemon(void){
    pid_t pid; // 用于存储子进程的标识
    int fd = 0;// 用于存储打开"/dev/null"文件的文件描述符

    // [1] 调用进程创建子进程A
    pid = fork();
    // 如果创建子进程失败
    if(pid < 0){
        perror("fork()");
        return -1;
    }

    // [2] 杀死调用进程，子进程调用setsid(2)创建一个新的会话
    if(pid > 0) exit(1);

     if(setsid() == (pid_t) -1){
        perror("setsid()");
        return -2;
     }

    // [3] 进程A创建子进程B
    pid = fork();
    // 如果创建子进程B失败
    if(pid < 0){
        perror("fork()");
        return -3;
    }

    // [4]杀死进程A，子进程B继续执行后续的步骤
    if(pid > 0) exit(1);

    // [5]文件屏蔽字要设置为0(因为脱离了终端<需要把uamsk设置为0>)
    umask(0);

    // [6]当前工作路径切换到"/"
    // 如果 把当前工作目录切换到根目录失败
    if(chdir("/") == -1){
        perror("chdir()");
        return -3;
    }

    // [7] 将文件描述符 0 1 2 重定向到 "/dev/null"
    fd = open("/dev/null", O_RDWR);//以读写的形式打开"/dev/null"
    // 如果判断打开文件是否失败
    if(fd < 0){
        perror("open()");//打印错误信息
        return -4;//由于打开文件失败,结束函数,并且返回-4
    }

    // [8] 把文件描述符 0 1 2 重定向到 "/dev/null"
    dup2(fd, 0);//把文件描述符0重定向到fd所指文件
    dup2(fd, 1);//把文件描述符1重定向到fd所指文件
    dup2(fd, 2);//把文件描述符2重定向到fd所指文件

    // 注意 : 加载成功不要关闭fd文件,否则会影响后续重定向的使用
    if(fd > 2) close(fd); // 如果fd大于2,说明fd文件没有被重定向到0 1 2，所以需要关闭fd文件
 
    return 0;
}

int main(void){
    FILE *fp = NULL;           // fp指针指向打开的"/tmp/out"文件的文件流
    int ret = 0;               // 存储错误码
    time_t tm;                 // 存储当前时间的时间戳
    struct tm *time_st = NULL; // time_st指针指向格式化时间的空间
    char buf[BUFSIZE] = {0};   // buf数组用来存储格式化时间的字符串

    // 调用我们自己封装的方法
    if(mydaemon() < 0){
        fprintf(stderr, "Mydaemon() Failed!\n");
        goto ERR_1;
    }

    fp = fopen("/tmp/out", "w"); // 以w的形式打开目标文件
    // 如果打开文件是否失败
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
        // 如果 把时间戳转换成格式化时间失败
        if((time_st = localtime(&tm)) == NULL){
            perror("localtime()");
            ret = -3;
            goto ERR_2;
        }
        memset(buf, 0, BUFSIZE);// 清空脏数据
        strftime(buf, BUFSIZE, "%Y年%m月%d日 %H:%M:%S\n", time_st); // 把格式化时间转换成格式化时间的字符串
        fputs(buf, fp); // 把buf存储的字符串写入到fp指针指向的文件流中
        fflush(NULL); // 刷新缓冲区
        sleep(1);
    }

ERR_2:
    fclose(fp);//关闭目标文件的文件流
ERR_1:
    return ret;
}
