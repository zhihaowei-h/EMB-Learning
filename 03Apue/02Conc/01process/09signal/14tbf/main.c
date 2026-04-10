/*使用系统调用IO函数模拟实现令牌桶模型*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "tbf.h"

#define BUFSIZE 16  // 
#define CPS BUFSIZE // 速率(积攒令牌和取走令牌的速率)
#define BURST (CPS) * 20 // 令牌桶中令牌数量的上限(尽量是速率的整数倍，将来判断的时候比较方便)

int td = 0; // 获取到的令牌库的下标

// 模拟 命令cat
static int mycat(int fd){   
    char buf[BUFSIZE] = {0}; // 存储读取到的数据
    int count = 0; // 存储成功读取到的字节数
    int n = 0; // 取走的令牌数(要读取的字节数)
    
    while(1){
        n = tbf_fetch_token(td, BUFSIZE); // 取走td令牌桶中的令牌
        memset(buf, 0, BUFSIZE); // 清空脏数据

        count = read(fd, buf, n); //从文件中读取数据(按照取走的令牌数量读取数据)

        // 如果读到了文件结尾的位置
        if(count == 0)
            break; // 跳出死循环
        else if(count < 0){ // 如果读取失败
            perror("read()");// 打印错误信息
            return -1; // 由于读取失败,结束函数,并且返回-1
        }
        write(1, buf, count); // 把数据写入到标准输出文件中
    }
}

int main(int argc, char *argv[]){
    int fd = 0;//fd变量用来保存打开文件的文件描述符
    
    // 如果命令行参数个数少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]); // 打印使用说明
        return -1; // 由于命令行参数个数少于2个,结束程序,并且返回-1
    }
    
    td = tbf_init(CPS, BURST);  // 初始化令牌桶

    // 如果初始化令牌桶失败
    if(td < 0){
        fprintf(stderr, "初始化令牌桶失败! 错误码 = %d\n", td);
        return -2;
    }
    
    fd = open(argv[1], O_RDONLY); // 通过open(2)以只读的形式打开文件
    
    // 判断打开文件是否失败
    if(fd < 0){
        perror("open()"); // 打印错误信息
        return -3; // 由于打开文件失败,结束程序,并且返回-2
    }
    
    mycat(fd); // 调用自己实现的mycat()

    close(fd); // 通过close(2)关闭文件

    tbf_destroy(td); // 销毁td令牌桶

    return 0;
}