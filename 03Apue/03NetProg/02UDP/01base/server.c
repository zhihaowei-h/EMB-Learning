// 接收端
#include "protocol.h"
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
/* 
接收端的功能:
1. 创建套接字
2. 将本地地址绑定到套接字
3. 接收数据包
4. 处理数据包
5. 关闭套接字
*/
int main(){
    struct sockaddr_in server_addr;// 定义一个sockaddr_in结构体变量server_addr，存储本地的地址信息
    // [1] 创建报式套接字(创建自己的邮箱)
    int sockfd; // 定义一个整型变量sockfd，存储套接字描述符
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // 设置协议族为IPv4，套接字类型为UDP，协议设置为默认值0
    // 如果 创建套接字失败
    if(sockfd == -1){
        perror("socket()");
        return -1;
    }

    // [2] 将本地地址与套接字绑定(把自己的邮箱地址写在邮箱上)
    server_addr.sin_family = AF_INET;            // 设置服务器地址结构体变量server_addr的协议族为IPv4
    inet_aton(SERVER_IP, &server_addr.sin_addr); // 将服务器IP地址从文本格式转换为二进制格式，并赋值给服务器地址结构体变量server_addr的sin_addr成员
    server_addr.sin_port = htons(SERVER_PORT);   // 将服务器端口号转换为网络字节序，并赋值给服务器地址结构体变量server_addr的sin_port成员
    // 使用bind函数绑定套接字和服务器地址，参数包括套接字描述符sockfd，服务器地址结构体变量server_addr
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("bind()");
        close(sockfd);
        return -2;
    }

    // [3] 接收数据包
    struct data_st buf; // 定义一个数据包结构体变量buf，存储接收到的数据
    struct sockaddr_in client_addr; // 定义一个sockaddr_in结构体变量client_addr，存储对方的地址信息
    socklen_t client_addr_len = sizeof(client_addr); // 定义一个socklen_t类型的变量client_addr_len，存储客户端地址结构体变量client_addr的大小
    ssize_t recv_len = 0; // 定义一个ssize_t类型的变量recv_len，存储接收数据包的长度
    while(1){
        // 使用recvfrom函数接收数据包，参数包括套接字描述符sockfd，数据包结构体变量buf的地址，数据包的大小，标志设置为0，客户端地址结构体变量client_addr的地址，以及客户端地址结构体变量client_addr的大小
        recv_len = recvfrom(sockfd, &buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len); // 
        // 如果 接收数据包失败
        if(recv_len == -1){
            perror("recvfrom()");
            close(sockfd);
            return -3;
        }
        printf("Received packet from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); // 打印接收到的数据包的来源地址和端口号
        printf("ID: %d, Message: %s\n", buf.id, buf.msg); // 打印接收到的数据包的通信id和消息内容
    }

    close(sockfd); // 关闭套接字

    return 0;
}