/*
需求: ./a.out-l-h-m-i
如果解析到了|选项,打印输出"breakfast"
如果解析到了-h选项,打印输出"lunch"
如果解析到了-m选项,打印输出"dinner"
如果解析到了-i选项,打印输出"supper”
*/
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *optstring = "lhim";//选项字符组成的字符串[可以根据自己的需求定义]
    int ret = 0;//ret变量用来接收返回值

    while(1)//循环解析命令行选项
    {
        ret = getopt(argc, argv, optstring);//解析命令行选项
        if(ret == -1)//判断是否解析完毕
            break;//由于解析完毕,跳出死循环
        switch(ret)
        {
            case 'l' : printf("breakfast\n"); break;
            case 'h' : printf("lunch\n"); break;
            case 'm' : printf("dinner\n"); break;
            case 'i' : printf("supper\n"); break;
            case '?' : printf("I Don't Know!\n"); break;
            default  : break;
        }
    }

    return 0;
}