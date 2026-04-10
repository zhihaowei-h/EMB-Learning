#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void)
{   
    int num = 10;//定义一个整型的变量,为了验证父子进程的独立性
    pid_t pid;//存储子进程的标识

    printf("调用fork(2)前,只有父进程(PID:%d),num = %d\n", getpid(), num);
    
    pid = fork();//创建子进程
    if(pid < 0)//判断创建子进程是否失败
    {
        perror("fork()");//打印错误信息
        exit(1);//由于创建子进程失败,终止进程,并且返回状态1
    }
    
    //======== fork(2)成功后,父子进程从这里开始分岔执行 ========
    if(pid == 0)//子进程的操作
    {
        num += 5;//子进程修改自己的num变量
        printf("I Am Child Process (PID:%d,PPID:%d) num = %d\n", getpid(), getppid(), num);
    }
    else//父进程的操作
    {
        sleep(30);//睡30s
        num -= 5;//父进程修改自己的num变量
        printf("I Am Parent Process (PID:%d,Child Process PID:%d) num = %d\n", getpid(), pid, num);
        //wait(NULL);//父进程等待子进程执行结束(收尸)
    }
    
    //父子进程都会执行这行代码(验证:同一份儿代码,父子进程各自执行一份儿)
    printf("PID:%d Game Over num = %d\n", getpid(), num);
    
    exit(0);
}
