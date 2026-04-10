#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define MIN 100
#define MAX 300

#define NUM (MAX - MIN + 1)

static int is_prime(int num)
{
    int i = 0;//循环变量

    sleep(1);

    if(num <= 1)//判断num是否小于等于1(是否不是质数)
        return 0;
    if(num == 2 || num == 3)
        return 1;

    for(i = 2; i <= num / i; i++)
    {
        if(num % i == 0)
            return 0;
    }
    return 1;
}

int main(void)
{
    int i = 0;//循环变量
    pid_t pid;//存储子进程的标识

    for(i = MIN; i <= MAX; i++)
    {
        pid = fork();//创建子进程
        if(pid < 0)//判断创建子进程是否失败
        {
            perror("fork()");//打印错误信息
            exit(1);//由于创建子进程失败,终止进程,并且返回状态1
        }
        if(pid == 0)//子进程的操作
        {
            if(is_prime(i))
                printf("%d Is A Prime Number\n", i);
            exit(0);//终止子进程,并且返回状态0
        }
    }

    for(i = 0; i < NUM; i++)
        wait(NULL);

    return 0;
}