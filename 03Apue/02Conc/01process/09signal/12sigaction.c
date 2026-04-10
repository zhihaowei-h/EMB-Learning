#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void sig_handler(int none){
    write(1, "!", 1);//往标准输出文件中写一个!
}

int main(void){   
    int i = 0, j = 0;//循环变量
    struct sigaction oldact, newact;      // 定义旧方案(备份)和新方案(配置)结构体变量

    // 初始化oldact方案(预设为系统默认配置)
    oldact.sa_handler = SIG_DFL;          // 将方案中的 预设信号处理函数(预设信号处理函数并不实际存在) 设置为 系统默认行为
    sigemptyset(&oldact.sa_mask);         // 将旧方案中的信号集清空(即旧方案不屏蔽任何信号)
    oldact.sa_flags = 0;                  // 没有特殊行为
    
    // 配置newact方案(自定义捕获配置)
    newact.sa_handler = sig_handler;      // 设置信号处理函数为自定义的 sig_handler(未来应用该方案的信号的行为默认就是这个)
    sigemptyset(&newact.sa_mask);         // 将newact方案中的信号集清空(即不屏蔽任何信号)
    sigaddset(&newact.sa_mask, SIGINT);   // 将newact方案中的信号集的SIGINT信号设置为屏蔽状态
    sigaddset(&newact.sa_mask, SIGRTMIN); // 将newact方案中的信号集的SIGRTMIN信号设置为屏蔽状态
    newact.sa_flags = 0;                  // 没有特殊行为
    
    // 正式将新方案中的行为应用到具体信号
    sigaction(SIGINT, &newact, NULL);    // SIGINT信号 采用 newact方案(就是告诉内核，以后SIGINT来了，就按newact方案跑: SIGINT的行为是newcat方案中的sa_handler，该信号是否被屏蔽要看newcat方案中的信号集，该信号是否要有特殊行为也要看newcat方案中的flag)
    sigaction(SIGRTMIN, &newact, NULL);  // SIGINT信号 采用 newact方案(就是告诉内核，以后SIGRTMIN来了，就按newact方案跑: SIGRTMIN的行为是newcat方案中的sa_handler，该信号是否被屏蔽要看newcat方案中的信号集，该信号是否要有特殊行为也要看newcat方案中的flag)
    
    // 正式更新进程的信号屏蔽字(这一步执行完后，咱们的信号集才算彻底设置完毕)
    sigprocmask(SIG_BLOCK, &newact.sa_mask, &oldact.sa_mask); // 把进程原来的信号屏蔽字存储到old中，再把newact新方案的信号集拷贝到进程的信号屏蔽字
    
    //在定义每一行内容期间不被SIGINT信号干扰
    for(i = 0; i < 10; i++){
        for(j = 0; j < 5; j++){
            write(1, "*", 1);//往标准输出文件中写一个"*"
            sleep(1);//睡1s
        }
        write(1, "\n", 1);//往标准输出文件中写一个"\n"
        sigsuspend(&oldact.sa_mask);// 当执行这一条语句相当于在执行
        /*当执行这一条语句相当于在执行
        [1]sigprocmask(SIG_SETMASK, &old, &set);
        [2]pause();
        [3]sigprocmask(SIG_SETMASK, &set, NULL);
        属于原子操作
        */
    }
    
    return 0;
}