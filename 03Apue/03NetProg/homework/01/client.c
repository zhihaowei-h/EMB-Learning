// 接收端(被动端)
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
#include <net/if.h>
#include "protocol.h"
/* 
[1] 创建套接字
[2] 绑定地址和端口
[3] 加入多播组
[4] 接收多播组中的消息
[5] 关闭套接字
*/
int main(int argc, char *argv[]){
    // [1] 创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1){
        perror("socket()");
        return -1;
    }

    // [2] 绑定地址和端口
    struct sockaddr_in local_addr;            // 定义一个sockaddr_in结构体变量local_addr，存储本地地址信息
    local_addr.sin_family = AF_INET;          // 将local_addr结构体变量的sin_family成员设置为AF_INET，表示使用IPv4协议
    inet_aton(LOCAL_IP, &local_addr.sin_addr);// 将本地IP地址转换为二进制格式，并赋值给local_addr的sin_addr成员
    local_addr.sin_port = htons(RECV_PORT);   // 将端口号转换为网络字节序，并赋值给local_addr的sin_port成员
    // 使用bind函数将套接字sockfd绑定到指定的地址和端口，参数包括套接字描述符sockfd，指向地址结构的指针(local_addr)，以及地址结构的大小
    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
        perror("bind()");
        return -1;
    }

    // [3] 加入多播组
    struct ip_mreqn mreq;                            // 定义一个ip_mreqn结构体变量mreq，存储多播组的相关信息
    inet_aton(MULTICAST_IP, &mreq.imr_multiaddr);    // 将本地字节序的多播组IP地址转换为网络字节序的二进制格式，并赋值给mreq的imr_multiaddr成员
    inet_aton(LOCAL_IP, &mreq.imr_address);          // 将本地IP地址转换为二进制格式，并赋值给mreq的imr_address成员
    mreq.imr_ifindex = if_nametoindex(NETCARD_NAME); // 获取网卡索引，并赋值给mreq的imr_ifindex成员
    // 使用setsockopt函数设置套接字选项，参数包括套接字描述符sockfd，协议级别IPPROTO_IP，选项名称IP_ADD_MEMBERSHIP，选项值为mreq结构体变量的地址，以及选项值的大小
    if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1){
        perror("setsockopt()");
        return -1;
    }

    // [4] 接收多播组中的消息
    char ptr[128] = {0};
    while(1){
        memset(ptr, 0, sizeof(ptr)); // 每次接收消息之前，先清空缓冲区，避免上次接收的消息残留在缓冲区中，影响本次接收的消息的正确性
        ssize_t recvlen = recvfrom(sockfd, ptr, sizeof(ptr) - 1, 0, NULL, NULL); // 留一个位置给 \0
        if (recvlen > 0) {
            ptr[recvlen] = '\0'; // 强制封口
            printf("msg = %s\n", ptr);
        }
        if(recvlen == -1){
            perror("recvfrom()");
            return -1;
        }else if(recvlen == 0){
            printf("recvfrom() returned 0, peer has performed an orderly shutdown\n");
            break;
        }
    }

    // [5] 关闭套接字
    close(sockfd);

    return 0;
}