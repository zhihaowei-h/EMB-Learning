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
/*
扩展实验：多频道、多网卡、单播混合发送
假设你的 server.c 现在的任务是：
组播 A (234.2.3.4): 发送普通日志，走业务网卡 ens33。
组播 B (224.1.1.1): 发送紧急告警，走备用网卡 ens36 (假设存在)。
单播 (192.168.1.100): 发送心跳包给特定管理员，走系统默认路由。

可以得出以下结论：
要指定两种组播策略
*/


int main(int argc, char *argv[]){
    struct ip_mreqn mreq;  // 给套接字指定的策略
    struct sockaddr_in addr_log;   // 普通日志的数据包的地址信息
    struct sockaddr_in addr_alarm; // 紧急告警的数据包的地址信息
    struct sockaddr_in addr_uni;   // 发送给管理员主机的数据包的地址信息
    
    // [1] 创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if(sockfd == -1){
        perror("socket()"); // 213
        return -1;
    }

    // 发送端不需要绑定地址和端口，系统会自动分配一个临时的端口号
    
    // 初始化第一种组播策略
    inet_aton(MULTICAST_IP1, &mreq.imr_multiaddr);
    inet_aton(LOCAL_IP, &mreq.imr_address);
    mreq.imr_ifindex = if_nametoindex(NETCARD_NAME1);
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) == -1){
        perror("setsockopt()");
        return -1;
    }

    inet_aton(MULTICAST_IP2, &mreq.imr_multiaddr);
    inet_aton(LOCAL_IP, &mreq.imr_address);
    mreq.imr_ifindex = if_nametoindex(NETCARD_NAME2);
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) == -1){
        perror("setsockopt()");
        return -2;
    }

    /* 现在sockfd就有两条关于组播的策略了
     未来通过sockfd发送数据时遵循以下规则:
    [1]如果发送的数据包的目的地是MULTICAST_IP1，那就通过NETCARD_NAME1这张网卡发出
    [2]如果发送的数据包的目的地是MULTICAST_IP2，那就通过NETCARD_NAME2这张网卡发出
    */ 


    // [3] 往多播组中发送消息
    addr_log.sin_family = AF_INET;
    inet_aton(MULTICAST_IP1, &addr_log.sin_addr);
    addr_log.sin_port = htons(RECV_PORT);

    addr_alarm.sin_family = AF_INET;
    inet_aton(MULTICAST_IP2, &addr_alarm.sin_addr);
    addr_alarm.sin_port = htons(RECV_PORT);

    addr_uni.sin_family = AF_INET;
    inet_aton("192.168.1.100", &addr_uni.sin_addr);
    addr_uni.sin_port = htons(RECV_PORT);
    while(1){
        sendto(sockfd, "log12", 5, 0, (struct sockaddr *)&addr_log, sizeof(addr_log));
        sendto(sockfd, "logso", 5, 0, (struct sockaddr *)&addr_alarm, sizeof(addr_alarm));
        sendto(sockfd, "hello", 5, 0, (struct sockaddr *)&addr_uni, sizeof(addr_uni));
        sleep(1);
    }

    // [4] 关闭套接字
    close(sockfd);
     
    return 0;
}