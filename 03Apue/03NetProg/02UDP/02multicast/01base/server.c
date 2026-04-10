// 发送端
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
#include <net/if.h>

int main(int argc, char *argv[]){
    struct ip_mreqn mreq;         // 给套接字指定的关于组播策略，当给套接字设置了组播策略后，这个套接字并不是只能发送组播数据包了，而是既可以发送组播数据包，也可以发送普通的单播数据包
    struct sockaddr_in group_addr;// 定义一个sockaddr_in结构体变量group_addr，存储多播组的地址信息
    
    // [1] 创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if(sockfd == -1){
        perror("socket()"); // 213
        return -1;
    }

    // 发送端不需要绑定地址和端口，系统会自动分配一个临时的端口号
    
    // [2] 将套接字使能为"组播就绪"状态
    inet_aton(MULTICAST_IP, &mreq.imr_multiaddr);   // 将多播组IP地址转换为二进制格式，并赋值给mreq的imr_multiaddr成员
    inet_aton(LOCAL_IP, &mreq.imr_address);         // 将本地IP地址转换为二进制格式，并赋值给mreq的imr_address成员
    mreq.imr_ifindex = if_nametoindex(NETCARD_NAME);// 获取网卡索引，并赋值给mreq的imr_ifindex成员
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) == -1){
        perror("setsockopt()");
        return -1;
    }

    // [3] 往多播组中发送消息
    group_addr.sin_family = AF_INET;               // 将group_addr结构体变量的sin_family成员设置为AF_INET，表示使用IPv4协议
    inet_aton(MULTICAST_IP, &group_addr.sin_addr); // 将多播组IP地址转换为二进制格式，并赋值给group_addr的sin_addr成员
    group_addr.sin_port = htons(RECV_PORT);        // 将接收端的端口号转换为网络字节序，并赋值给group_addr的sin_port成员
    while(1){
        sendto(sockfd, "hello", 5, 0, (struct sockaddr *)&group_addr, sizeof(group_addr));
        sleep(1);
    }

    // [4] 关闭套接字
    close(sockfd);
     
    return 0;
}