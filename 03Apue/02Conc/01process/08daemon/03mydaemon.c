#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 128

static int mydaemon(void)
{   
    pid_t pid;//存储子进程的标识
    int fd = 0;//存储打开"/dev/null"文件的文件描述符
    
    pid = fork();//创建子进程
    if(pid < 0)//判断创建子进程是否失败
    {   
        perror("fork()");//打印错误信息
        return -1;//由于创建子进程失败,结束函数,并且返回-1
    }
    
    if(pid > 0)//父进程的操作
        _exit(0);//终止父进程(不需要调用终止处理函数)
    
    if(setsid() == (pid_t) -1)//判断创建一个新的会话是否失败
    {   
        perror("setsid()");//打印错误信息
        return -2;//由于创建一个新的会话失败,结束函数,并且返回-2
    }
    
    //执行到此,当前子进程就是新的会话新的进程组的组长进程
    //此时,子进程的PID == PGID == SID,而且脱离了控制终端
    //往下可以继续做[4] [5] [6]的操作(做了会更好)
    
    //[4]文件屏蔽字要设置为0(因为脱离了终端<需要把uamsk设置为0>)
    umask(0);
    
    //[5]当前工作路径切换到"/"
    if(chdir("/") == -1)//判断把当前工作目录切换到根目录是否失败
    {
        perror("chdir()");//打印错误信息
        return -3;//由于把当前工作目录切换到根目录失败,结束函数,并且返回-3
    }
    
    //[6]将文件描述符 0 1 2 重定向到 "/dev/null"
    fd = open("/dev/null", O_RDWR);//以读写的形式打开"/dev/null"
    if(fd < 0)//判断打开文件是否失败
    {
        perror("open()");//打印错误信息
        return -4;//由于打开文件失败,结束函数,并且返回-4
    }
    
    dup2(fd, 0);//把文件描述符0重定向到fd文件
    dup2(fd, 1);//把文件描述符1重定向到fd文件
    dup2(fd, 2);//把文件描述符2重定向到fd文件
    
    if(fd > 2)
        close(fd);
    
    return 0;
}

int main(void)
{   
    FILE *fp = NULL;//fp指针指向打开的"/tmp/out"文件的文件流
    int ret = 0;//ret存储错误码
    time_t tm;//存储当前时间的时间戳
    struct tm *time_st = NULL;//time_st指针指向格式化时间的空间
    char buf[BUFSIZE] = {0};//buf数组用来存储格式化时间的字符串

#if 0
    if(mydaemon() < 0)//调用我们自己封装的方法
    {
        fprintf(stderr, "Mydaemon() Failed!\n");//打印错误信息
        goto ERR_1;//跳转到ERR_1的标志
    }
#else
    // 如果 将调用进程变成守护进程失败
    if(daemon(0, 0) == -1){
        perror("daemon()");
        goto ERR_1;
    }
#endif
    
    fp = fopen("/tmp/out", "w");//以w的形式打开目标文件
    // 如果打开文件失败
    if(fp == NULL){
        perror("fopen()");
        ret = -1;
        goto ERR_1;
    }

    //死循环
    while(1){
        if(time(&tm) == (time_t)-1)//判断获取当前时间的时间戳是否失败
        {
            perror("time()");//打印错误信息
            ret = -2;//设置错误码为-2
            goto ERR_2;//跳转到ERR_2的标志
        }
        if((time_st = localtime(&tm)) == NULL)//判断把时间戳转换成格式化时间是否失败
        {
            perror("localtime()");//打印错误信息
            ret = -3;//设置错误码为-3
            goto ERR_2;//跳转到ERR_2的标志
        }
        //memset();//清空脏数据
        strftime(buf, BUFSIZE, "%Y年%m月%d日 %H:%M:%S\n", time_st);
        //把格式化时间转换成格式化时间的字符串
        fputs(buf, fp);//把buf存储的字符串写入到fp指针指向的文件流中
        fflush(NULL);//刷新缓冲区
        sleep(1);
    }

ERR_2:
    fclose(fp);//关闭目标文件的文件流
ERR_1:
    return ret;
}