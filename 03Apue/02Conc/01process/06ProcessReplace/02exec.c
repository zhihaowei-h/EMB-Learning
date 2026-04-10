#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void){
    pid_t pid; //存储子进程的标识
    char *str[] = {"ls", "-l", NULL}; // 把选项参数列表合到了一起

    printf("Begin...\n");
    
    pid = fork();//创建子进程
    // 如果 创建子进程失败
    if(pid < 0){
        perror("fork()");
        exit(1); // 由于创建子进程失败,终止进程,并且返回状态1
    }
    
    // 子进程的操作
    if(pid == 0){
        // execl("/usr/bin/ls", "ls", "-l", NULL);//使用 "ls -l" 替换子进程
        // execlp("ls", "ls", "-l", NULL);
        // execle("/usr/bin/ls", "ls", "-l", NULL, NULL);
        // execv("/usr/bin/ls", str); // 这5个都是等价的
        execvp("ls", str); // 这3个都是等价的
        perror("exec()"); // 如果exec函数返回，说明替换失败，输出错误信息
        exit(2); // 由于进程替换失败，终止子进程进程，并且返回状态2
    }
    
    wait(NULL);// 等待子进程退出
    printf("End...\n");
    
    return 0;
}
