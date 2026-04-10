/*使用系统调用IO函数模拟实现漏桶模型*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define BUFSIZE 128

static int flag = 0; // 用来标记是否调用进程执行IO操作(0: 没有执行 1: 正在执行)

// SIGALRM信号的信号处理函数
static void sig_handler(int none){
    alarm(1); // 设置1s的闹钟
    flag = 1; // 设置flag的标记为1
}

// 
static int mycat(int fd){
    char buf[BUFSIZE] = {0}; // 存储读取到的数据
    int count = 0; // 存储成功读取到的字节数
    
    while(1){
        while(!flag); // 死等，直到flag==1才往下执行(忙等/死等会导致单核CPU占用率接近100%)
        flag = 0; // 恢复标记的初始值
        memset(buf, 0, BUFSIZE); // 清空脏数据
        count = read(fd, buf, BUFSIZE); // 从文件中读取数据

        // 如果 读到了文件的末尾
        if(count == 0){
            break;
        }else if(count < 0){ // 如果读取失败
            perror("read()");
            return -1;
        }
        write(1, buf, count); // 把数据写入到标准输出文件中
    }
}
 
int main(int argc, char *argv[]){   
    int fd = 0; // fd变量用来保存打开文件的文件描述符

    // 如果 命令行参数个数少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]); // 打印使用说明
        return -1;
    }
    
    signal(SIGALRM, sig_handler); // 给SIGALRM信号设置新行为
    alarm(1); // 给SIGALRM信号设置1s的闹钟，1s后会向该调用进程发送SIGALRM信号
    
    fd = open(argv[1], O_RDONLY); // 通过open(2)以只读的形式打开文件

    // 如果 文件打开失败
    if(fd < 0){
        perror("open()"); // 打印错误信息
        return -2;
    }
    
    mycat(fd); // 调用自己实现的mycat()

    close(fd); // 通过close(2)关闭文件

    return 0;
}