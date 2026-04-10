#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void sig_handler(int none){
    write(1, "!", 1); // 往标准输出 中写一个!
}

int main(void){
    int i = 0, j = 0;//循环变量
    sigset_t old, new;//old存储原有的信号集,new存储设置的信号集

    sigemptyset(&new);//把new信号集清空
    sigaddset(&new, SIGINT);//将SIGINT信号添加到new信号集中
    sigaddset(&new, SIGRTMIN);//将SIGRTMIN信号添加到new信号集中

    signal(SIGINT, sig_handler);//给SIGINT信号设置新的行为
    signal(SIGRTMIN, sig_handler);//给SIGRTMIN信号设置新的行为

    //给当前调用进程阻塞new信号集中的信号,并且把原有的信号屏蔽字存储到old中
    sigprocmask(SIG_BLOCK, &new, &old);

    //在定义每一行内容期间不被SIGINT信号干扰
    for(i = 0; i < 10; i++){
        for(j = 0; j < 5; j++){
            write(1, "*", 1); // 往标准输出中写一个"*"
            sleep(1); // 睡1s
        }
        write(1, "\n", 1);// 往标准输出文件中写一个"\n"

        sigsuspend(&old);  // 进程会停在这里
        /*当执行这一条语句相当于在执行
        [1]sigprocmask(SIG_SETMASK, &old, &set); // 将进程的信号屏蔽字拷贝到set中，再将进程的信号屏蔽字设置为old   
        [2]pause(); // 等待信号到来: 如果是被屏蔽的信号先放进未决，并不会唤醒进程；如果是不被屏蔽的信号，放进未决，然后唤醒进程 // 竞态条件风险：如果在执行完 SIG_SETMASK 之后，还没来得及执行 pause() 的那一瞬间，信号刚好到了并处理完了，那么 pause() 就会一直阻塞在那里，因为它错过了信号。
        [3]sigprocmask(SIG_SETMASK, &set, NULL); // 将进程的信号屏蔽字恢复成原来的set
        属于原子操作
        */
    }

    return 0;
}
