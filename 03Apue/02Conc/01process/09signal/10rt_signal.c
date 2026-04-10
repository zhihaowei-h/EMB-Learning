#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void sig_handler(int none){
    write(1, "!", 1);//往标准输出中写一个!
}

int main(void)
{
    int i = 0, j = 0;//循环变量
    sigset_t old, new;//old存储原有的信号集,new存储设置的信号集

    sigemptyset(&new);//把new信号集清空
    sigaddset(&new, SIGINT);//将SIGINT信号添加到new信号集中
    sigaddset(&new, SIGRTMIN);//将SIGRTMIN信号添加到new信号集中

    signal(SIGINT, sig_handler);//给SIGINT信号设置新的行为
    signal(SIGRTMIN, sig_handler);//给SIGRTMIN信号设置新的行为

    //在定义每一行内容期间不被SIGINT信号干扰
    for(i = 0; i < 10; i++){
        //给当前调用进程阻塞new信号集中的信号,并且把原有的信号屏蔽字存储到old中
        sigprocmask(SIG_BLOCK, &new, &old);
        for(j = 0; j < 5; j++){
            write(1, "*", 1);//往标准输出文件中写一个"*"
            sleep(1);//睡1s
        }
        write(1, "\n", 1);//往标准输出文件中写一个"\n"
        sigprocmask(SIG_SETMASK, &old, NULL);
        //给当前调用进程恢复之前的信号屏蔽字
    }

    return 0;
}