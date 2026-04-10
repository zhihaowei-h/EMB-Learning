// 发送端
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "protocol.h"
/* 
把argvp[1]当作IP
把argvp[2]当作PORT
把argvp[3]当作ID
把argvp[4]当作消息内容
*/
int main(int argc, char *argv[]){
    // 如果命令行参数少于5个
    if(argc < 5){
        fprintf(stderr, "Usage: %s <ip> <port> <id> <msg>\n", argv[0]);
        return -1;
    }

    // [1] 创建报式套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // 设置协议族为IPv4，套接字类型为UDP，协议设置为默认值0
    // 如果 创建套接字失败
    if(sockfd == -1){
        perror("socket()");
        return -2;
    }

    // [2] 整理数据包(邮件)
    struct data_st buf;                   // 定义一个数据包结构体变量buf，存储要发送的数据
    buf.id = atoi(argv[3]);               // 将命令行参数argv[3]转换为整数，并赋值给数据包结构体变量buf的id成员
    strncpy(buf.msg, argv[4], MSGSIZE-1); // 将命令行参数argv[4]复制到数据包结构体变量buf的msg成员中，确保不超过MSGSIZE-1个字符
    buf.msg[MSGSIZE-1] = '\0';            // 确保字符串以空字符结尾
    
    // [3] 在邮件封面写地址一栏，明确自己是谁(网络编程中就是明确本地的地址(IP + PORT)，以便对方回信)
    struct sockaddr_in server_addr;              // 定义一个sockaddr_in结构体变量server_addr，存储本地的地址信息(IP + PORT)
    server_addr.sin_family = AF_INET;            // 设置本地地址结构体变量server_addr的协议族为IPv4
    inet_aton(argv[1], &server_addr.sin_addr);   // 将命令行参数argv[1]转换为二进制格式，并赋值给服务器地址结构体变量server_addr的sin_addr成员，为后面发送数据包时强转为sockaddr结构体做准备
    server_addr.sin_port = htons(atoi(argv[2])); // 将命令行参数argv[2]端口转换为整数并转换为网络字节序，赋值给服务器地址结构体变量server_addr的sin_port成员，为后面发送数据包时强转为sockaddr结构体做准备
    
    // [4] 发送数据包(发送邮件)
    // 使用sendto函数发送数据包，参数包括套接字描述符sockfd，数据包结构体变量buf的地址，数据包的大小（id成员的大小加上消息字符串的长度加1），标志设置为0，服务器地址结构体变量server_addr的地址，以及服务器地址结构体变量server_addr的大小
    if(sendto(sockfd, &buf, sizeof(buf.id) + strlen(buf.msg) + 1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){ 
        perror("sendto()");
        close(sockfd);
        return -3;
    }

    // [5] 关闭套接字(关闭邮箱)
    close(sockfd);

    return 0;
}