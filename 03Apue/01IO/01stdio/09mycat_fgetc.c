/*使用fgetc-fputc实现mycat的命令*/
#include <stdio.h>

static int mycat(const char *pathname)
{
    FILE *fp = NULL;//fp指针指向打开的文件流
    int ch = 0;//ch变量存储从文件流中读取的字符

    fp = fopen(pathname, "r");//通过fopen(3)以r的方式打开文件
    if(fp == NULL)//判断打开文件是否失败
    {
        perror("fopen()");//打印错误信息
        return -1;//由于打开文件失败,结束函数,并且返回-1
    }

    while(1)//循环读写文件
    {
        ch = fgetc(fp);//从fp指向的文件流中读取一个字符
        if(ch == EOF)//判断是否读取到了EOF
        {//注意 : 当读到了文件结尾会返回EOF;当读取错误也会返回EOF
            if(ferror(fp))//判断fp指向的文件流是否出错
            {
                fclose(fp);//关闭fp指向的文件流
                return -2;//由于fp指向的文件流出错,结束函数,并且返回-2
            }
            break;//读到了文件结尾,跳出死循环
        }
        fputc(ch, stdout);//把ch存储的字符数据写入到stdout中
    }
    fclose(fp);//关闭fp指向的文件流
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2)//判断命令行参数的个数是否少于2个
    {
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);//打印使用说明
        return -1;//由于令行参数的个数少于2个,结束程序,并且返回-1
    }

    mycat(argv[1]);//调用实现的内部函数,完成cat的功能

    return 0;
}