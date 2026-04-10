#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static void handler(int none)//注册的新行为(当SIGINT信号到来了,会调用handler函数)
{
    int i = 0;//循环变量

    for(i = 0; i < 3; i++)
    {
        write(1, "!", 1);//往标准输出写一个"!"
        sleep(1);
    }
}

int main(void)
{
    int i = 0;//循环变量

    if((signal(SIGINT, handler)) == SIG_ERR)//判断给SIGINT信号设置新的行为是否失败
    {
        perror("signal()");//打印错误信息
        return -1;//由于给SIGINT信号设置新的行为失败,结束程序,并且返回-1
    }

    while(1)//死循环,每1s往标准输出写一个"*"
    {
        write(1, "*", 1);//往标准输出写一个"*"
        sleep(1);//睡1s
        i++;//循环变量自增
        if(i == 10)//判断是否到了10s
            signal(SIGINT, SIG_DFL);//给SIGINT信号设置默认行为
    }

    return 0;
}
