/*使用系统调用IO函数实现mycat*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DUP2

#define BUFSIZE 128

static int mycat(int fd)
{
    char buf[BUFSIZE] = {0};//存储读取到的数据
    int count = 0;//存储成功读取到的字节数

    while(1)
    {
        memset(buf, 0, BUFSIZE);//清空脏数据
        count = read(fd, buf, BUFSIZE);//从文件中读取数据
        if(count == 0)//判断是否读到了文件结尾的位置
            break;//跳出死循环
        else if(count < 0)//判断是否读取失败
        {
            perror("read()");//打印错误信息
            return -1;//由于读取失败,结束函数,并且返回-1
        }
        write(1, buf, count);//把数据写入到标准输出文件中
    }
}

int main(int argc, char *argv[])
{
    int fd = 0;//fd变量用来保存打开文件的文件描述符
    int fdo = 0;//fdo变量用来保存输出文件的文件描述符

    if(argc < 3)//判断命令行参数个数是否少于3个
    {
        fprintf(stderr, "Usage : %s + srcname + destfile\n", argv[0]);//打印使用说明
        return -1;//由于命令行参数个数少于3个,结束程序,并且返回-1
    }

    fd = open(argv[1], O_RDONLY);//通过open(2)以只读的形式打开文件
    if(fd < 0)//判断打开文件是否失败
    {
        perror("open()");//打印错误信息
        return -2;//由于打开文件失败,结束程序,并且返回-2
    }

    fdo = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);//通过open(2)打开输出文件
    if(fdo < 0)//判断打开输出文件是否失败
    {
        perror("open()");//打印错误信息
        close(fd);//关闭fd文件
        return -3;//由于打开输出文件失败,结束程序,并且返回-3
    }

#if defined(DUP)
    //不是原子操作
    close(1);//关闭1号文件描述符
    dup(fdo);//把当前可用最小的文件描述符(1号)作为fdo的复制
#elif defined(DUP2)
    //原子操作
    dup2(fdo, 1);//手动指定1号文件描述符作为fdo的复制
#endif

    mycat(fd);//调用自己实现的mycat()

    close(fdo);//通过close(2)关闭输出文件
    close(fd);//通过close(2)关闭文件

    return 0;
}
