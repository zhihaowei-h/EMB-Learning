#include <stdio.h>

int main(void)
{
    FILE *fp = NULL;//fp指针指向打开的文件

    fp = fopen("abc", "r");//通过fopen(3)以读的形式打开abc文件
    if(fp == NULL)//判断打开文件是否失败
    {
        perror("fopen()");//打印错误信息
        return -1;//由于打开文件失败,结束程序,并且返回-1
    }

    printf("fp = %p\n", fp);

    fclose(fp);//关闭文件
    fp = NULL;//防止野指针的出现

    return 0;
}