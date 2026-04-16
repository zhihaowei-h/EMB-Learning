// 目标: 编写一个守护进程,要求这个守护进程只能有一个实例在运行,并且每隔1秒钟就把当前的时间写入到"/tmp/out"文件中
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#define BUFSIZE 128
#define LOCKFILE "/var/run/mydaemon.pid" // 锁文件的路径，一般放在/var/run目录下,因为这个目录是专门用来存放运行时文件的,而且这个目录的权限是root用户可写的,所以可以保证只有root用户才能创建锁文件,从而保证了守护进程的安全性
//#define LOCKFILE "/tmp/mydaemon.pid"

// 封装一个 将子进程变成守护进程 的函数
static int mydaemon(void){
    pid_t pid; // 存储子进程的标识
    int fd = 0; // 存储打开"/dev/null"文件的文件描述符
    
    pid = fork(); // 创建子进程
    // 如果 创建子进程失败
    if(pid < 0){
        perror("fork()");
        return -1;
    }
    
    // 如果是父进程，直接杀死，因为父进程的存在会影响到子进程，而我们希望守护进程是独立的，所以父进程没有存在的意义了
    if(pid > 0) _exit(0);

    // 如果是子进程, 将其变成守护进程
    // 如果创建一个新的会话失败
    if(setsid() == (pid_t) -1){
        perror("setsid()");
        return -2;
    }
    
    /*
    执行到此,当前子进程就是新的会话新的进程组的组长进程
    此时,子进程的PID == PGID == SID,而且脱离了控制终端
    往下可以继续做[4] [5] [6]的操作(做了会更好)
    */
    
    // [4] 文件屏蔽字要设置为0(因为脱离了终端<需要把uamsk设置为0>)
    umask(0);
    
    // [5] 当前工作路径切换到"/"
    // 如果把当前工作目录切换到根目录失败
    if(chdir("/") == -1){
        perror("chdir()");//打印错误信息
        return -3;//由于把当前工作目录切换到根目录失败,结束函数,并且返回-3
    }
    
    //[6]将文件描述符 0 1 2 重定向到 "/dev/null"
    fd = open("/dev/null", O_RDWR); // 以读写的形式打开"/dev/null"
    // 如果打开文件失败
    if(fd < 0){
        perror("open()");
        return -4;
    }
    dup2(fd, 0);//把文件描述符0重定向到fd文件
    dup2(fd, 1);//把文件描述符1重定向到fd文件
    dup2(fd, 2);//把文件描述符2重定向到fd文件
    
    // 如果fd大于2，说明fd不是0 1 2中的一个，所以要关闭fd文件
    if(fd > 2) close(fd);

    return 0;
}

// 封装一个判断是否已经有一个守护进程在运行的函数，供后续调用
static int process_already_running(void){
    int fd = 0; // 存储打开文件的文件描述符
    char buf[BUFSIZE] = {0}; // 存储获取到锁文件进程的PID字符串

    fd = open(LOCKFILE, O_WRONLY | O_CREAT, 0666);//以只写的形式打开或者创建文件
    // 如果打开或者创建文件失败
    if(fd < 0){
        perror("open()");
        return -1;
    }

    if(flock(fd, LOCK_EX | LOCK_NB) == -1)//判断给文件加非阻塞互斥锁是否失败
    {
        perror("flock()");//打印错误信息
        close(fd);//关闭锁文件
        return -2;//由于给文件加非阻塞互斥锁失败,结束函数,并且返回-2
    }
    ftruncate(fd, 0); // 把锁文件截断为0,如果锁文件之前有内容,就把它清空,如果锁文件之前没有内容,就不做任何操作
    // 如果把锁文件截断为0失败
    if(ftruncate(fd, 0) == -1){
        perror("ftruncate()");
        close(fd);
        return -3;
    }

    sprintf(buf, "%d", getpid()); // 把获取到锁文件的进程的PID转换成字符串
    write(fd, buf, strlen(buf)); // 把获取到锁文件的进程的PID的字符串写入锁文件中

    //注意 : 加载成功不要关闭锁文件,否则会影响后续锁文件的获取

    return 0;
}

int main(void){
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
    if(daemon(0, 0) == -1)//判断创建守护进程是否失败
    {
        perror("daemon()");//打印错误信息
        goto ERR_1;//跳转到ERR_1的标志
    }
#endif

    if(process_already_running() < 0)//判断给文件加非阻塞互斥锁是否失败
    {
        fprintf(stderr, "process_already_running() Is Failed!\n");//打印错误信息
        goto ERR_1;//跳转到ERR_1的标志
    }

    fp = fopen("/tmp/out", "w");//以w的形式打开目标文件
    if(fp == NULL)//判断打开文件是否失败
    {
        perror("fopen()");//打印错误信息
        ret = -1;//设置错误码为-1
        goto ERR_1;//跳转到ERR_1的标志
    }

    while(1)//死循环
    {
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
