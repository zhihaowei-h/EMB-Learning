// 发送端
#include <stdio.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include "protocol.h"



int main(int argc, char *argv[]){
    
    // 命令 <IP地址> <端口号> <state.id> <message>
    // 如果 命令行参数少于5个
    if(argc < 5){
        fprintf(stderr, "Usage");
        return -1;
    }
    
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // 如果创建套接字失败
    if(sockfd == -1){
        perror("socked()");
        return -2;
    }

    // 整理数据包
    struct data mydata;
    mydata.data_id = atoi(argv[3]);
    // mydata.str = argv[4];   // 为什么不能直接赋值: 数组名是常量指针，不能被赋值
    // 解决方法1：使用strncpy函数复制字符串
    strncpy(mydata.str, argv[4], MSGSIZE-1); // 确保不超过MSGSIZE-1个字符
    mydata.str[MSGSIZE-1] = '\0'; // 确保字符串以空字符结尾


    // FIXME: 到此










    return 0;
}