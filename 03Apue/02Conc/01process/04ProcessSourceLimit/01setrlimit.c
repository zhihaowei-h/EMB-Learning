#include <stdio.h>        // 标准输入输出的头文件
#include <time.h>         // 时间相关的头文件
#include <sys/resource.h> // 资源限制的头文件
  
  
int main(void){
    struct rlimit limit; // 定义一个 rlimit 结构体变量, 用于存储进程资源限制的信息
    /*
    struct rlimit {
        rlim_t rlim_cur; // 当前资源限制的值
        rlim_t rlim_max; // 资源限制的最大值
    };
    */
  
    // 判断获取进程资源限制的信息是否失败
    if (getrlimit(RLIMIT_STACK, &limit) != 0) { // 获取当前进程的栈区大小限制
        perror("getrlimit()"); // 如果获取失败，输出错误信息
        return -1;             // 返回非零值表示程序异常结束
    }
  
    printf("Stack Limit: %ldKB\n", limit.rlim_cur >> 10); // 输出当前进程的栈区大小限制，单位为KB
    return 0;
}