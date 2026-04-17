/*通过 open(2) close(2) 实现mytouch命令*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd = 0;//fd变量用来接收新创建文件的文件描述符
    int i = 0;//循环变量

    if(argc < 2)//判断命令行参数的个数是否少于2个
    {
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);//打印使用说明
        return -1;//由于命令行参数的个数少于2个,结束程序,并且返回-1
    }

    for(i = 1; i < argc; i++)//循环创建文件
    {
        fd = open(argv[i], O_WRONLY | O_CREAT, 0666);//通过open(2)打开文件
        if(fd < 0)//判断打开文件是否失败
        {
            perror("open()");//打印错误信息
            return -2;//由于打开文件失败,结束程序,并且返回-2
        }
        close(fd);//通过close(2)关闭文件
    }

    return 0;
}