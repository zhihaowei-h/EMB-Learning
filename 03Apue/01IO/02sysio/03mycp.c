/*使用系统调用IO函数实现mycp*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 128

static int mycp(int fds, int fdd)
{
    char buf[BUFSIZE] = {0};//存储读取到的数据
    int count = 0;//存储成功读取到的字节数

    while(1)
    {
        memset(buf, 0, BUFSIZE);//清空脏数据
        count = read(fds, buf, BUFSIZE);//从源文件中读取数据
        if(count == 0)//判断是否读到了文件结尾的位置
            break;//跳出死循环
        else if(count < 0)//判断是否读取失败
        {
            perror("read()");//打印错误信息
            return -1;//由于读取失败,结束函数,并且返回-1
        }
        write(fdd, buf, count);//把数据写入到目标文件中
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fds = 0;//fds变量用来保存源文件的文件描述符
    int fdd = 0;//fdd变量用来保存目标文件的文件描述符

    if(argc < 3)//判断命令行参数个数是否少于3个
    {
        fprintf(stderr, "Usage : %s + srcname + destname\n", argv[0]);//打印使用说明
        return -1;//由于命令行参数个数少于3个,结束程序,并且返回-1
    }

    fds = open(argv[1], O_RDONLY);//通过open(2)以只读的形式打开源文件
    if(fds < 0)//判断打开源文件是否失败
    {
        perror("open()");//打印错误信息
        return -2;//由于打开源文件失败,结束程序,并且返回-2
    }

    fdd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);//通过open(2)打开目标文件
    if(fdd < 0)//判断打开目标文件是否失败
    {
        close(fds);//由于打开目标文件失败,关闭源文件
        perror("open()");//打印错误信息
        return -3;//由于打开目标文件失败,结束程序,并且返回-3
    }

    mycp(fds, fdd);//调用自己实现的mycp()

    close(fdd);//通过close(2)关闭目标文件
    close(fds);//通过close(2)关闭源文件

    return 0;
}
