// 把23行和33行彻底看明白，信号集就彻底easy了
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// 信号SIGINT的信号处理函数
static void sig_handler(int none){
    write(1, "!", 1); // 往标准输出文件中写一个'!'
}

int main(void){
    int i = 0, j = 0; // 循环变量
    sigset_t new, old; // new存储待设置的信号集， old存储原有的信号集

    sigemptyset(&new); // 把new信号集清空
    sigaddset(&new, SIGINT); // 将SIGINT信号添加到new信号集中

    signal(SIGINT, sig_handler); // 给SIGINT信号设置新的行为

    // 在定义每一行内容期间不被SIGINT信号干扰
    for(i = 0; i < 10; i++){
        // 帮当前调用进程屏蔽new信号集中的信号，并且把原有的信号屏蔽字存储到old中
        sigprocmask(SIG_BLOCK, &new, &old); // 这行在执行时，会先把内核中的信号屏蔽字拷贝到old指向的空间中，然后内核中的信号屏蔽字再更新(有屏蔽new信号集中的信号的规则了)，但是old并不知道要屏蔽new信号集中的信号，因为old保存的是将new信号集设置为屏蔽信号之前的信号屏蔽字
        
        // 每隔1s写1个'*'，执行5次
        for(j = 0; j < 5; j++){
            write(1, "*", 1); // 往标准输出中写一个"*"
            sleep(1); // 睡1s
        }

        write(1, "\n", 1); // 往标准输出中写一个"\n"

        sigprocmask(SIG_SETMASK, &old, NULL); // 给当前调用进程恢复成之前的信号屏蔽字(恢复成之前的规则)，因为之前被屏蔽的信号的在pending中对应的位是1，mask对应的位是1(此时这个信号的状态就是已经到达这个进程了，但是还不能执行)，这行代码执行完后，mask对应的位就恢复成0了(此时这个信号的状态就是已经到达这个进程了，等待被执行)，所以这行代码执行完后恢复成用户态时，该信号的信号处理函数就执行了
    }

    return 0;
}