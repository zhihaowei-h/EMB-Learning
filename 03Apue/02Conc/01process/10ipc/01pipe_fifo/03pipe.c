// 父进程读argv[1]文件，写进管道，然后子进程从管道中读，再写到标准输出中
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>

#define SIZE 128 // 存储每次读取的字节大小

int main(int agrc, char *argv[]){
    int ret = 0; // 存储函数调用的返回值
    char buf[SIZE] = {0}; // 存储从管道中读取的数据
    int fd = 0; // 记录打开的文件的文件描述符
    pid_t pid;  // 进程ID
    int pfd[2] = {0}; // 存储管道的两个文件描述符，fd[0]用于读，fd[1]用于写

    // 如果命令行参数少于2个
    if(agrc < 2){
        fprintf(stderr, "Usage: ./a.out <pathname>\n");
        ret = -1;
        goto ERR_ARGC_1;
    }

    // 创建管道
    if(pipe(pfd)){
        perror("pipe()");
        ret = -2;
        goto ERR_PIPE_2;
    }

    /* 创建子进程
    当调用fork()时，如果成功的话，内核会瞬间克隆出一个一模一样的子进程，自此开始分流。父进程的PID会被返回子进程的实际ID，而子进程中的PID会被内
    核设置为0(表示为该父进程的第0个进程)。子进程和父进程一样，也从该行代码之后开始执行(也就是说，如果fork()成功后，父进程和子进程都会尝试执行
    后面的代码): 
    ①若创建失败，不会有子进程，fork给父进程的pid变量赋值为-1，表示我没能给你创建成功
    ②若创建成功
        父进程中的PID就是子进程的实际ID(肯定大于0，所以会跳过if(pid == 0)，那么在父进程中，就会跳过if(pid == 0)
        子进程中的PID是内核设置的编号(这个编号记录的是该子进程是其父进程的第几个子进程)，为0；子进程当然会去if(pid == -1),但是一定进不去，子进
        程会进入if(pid == 0)，进入if(pid == 0)后就被杀死了，被杀死了就不会去执行后续的代码了
    */
    pid = fork();
    // 如果 创建子进程失败
    if(pid == -1){
        perror("fork()");
        ret = -3;
        goto ERR_FORK_3;
    }

    // 子进程操作
    if(pid == 0){
        close(pfd[1]); // 从管道中读，关闭写端的文件描述符
        read(pfd[0], buf, SIZE); // 如是空管道会一直在这里阻塞(注意: 这里的buf不是父进程的buf，当fork成功后，子进程会拷贝父进程的虚拟地址空间，所以子进程也有了buf这个数组，但是要注意，父进程对自己的buf的操作并不会影响子进程的buf，所以这里的buf应该和父进程fork之前的buf是一样的，这里应该就全是0)
        close(pfd[0]);
        printf("Child: %s\n", buf);
        exit(0);
    }

    
    // 父进程操作
    fd = open(argv[1], O_RDONLY);
    // 如果父进程打开文件失败
    if(fd == -1){
        perror("open()");
        ret = -4;
        goto ERR_OPEN_4;
    }
    read(fd, buf, SIZE);
    close(pfd[0]); // 关闭父进程的往管道的读端的文件描述符
    write(pfd[1], buf, SIZE);
    close(pfd[1]);
    wait(NULL);
    return 0;

ERR_OPEN_4:
    close(fd);
ERR_FORK_3:
    close(pfd[0]);
    close(pfd[1]);
ERR_PIPE_2:
ERR_ARGC_1:
    return ret;
}