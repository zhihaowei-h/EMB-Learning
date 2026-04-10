// 主动端(发送端)
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
#include <fcntl.h>

int main(int argc, char *argv[]){
    struct ip_mreqn mreq;         // 用于存放组播组地址和本地网卡信息
    struct sockaddr_in group_addr;// 用于存储 组播组的地址信息

    int fp = open("/etc/passwd", O_RDONLY);
    if(fp == -1){
        perror("open()");
        return -1;
    }
    char buf[128] = {0};
    
    // [1] 创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if(sockfd == -1){
        perror("socket()");
        return -1;
    }

    // 发送端不需要绑定地址和端口，系统会自动分配一个临时的端口号
    
    // [2] 使能组播
    // 1. 设置目标组播组的 IP（虽然在发送接口设置中通常不生效，但建议保持逻辑完整）
    inet_aton(MULTICAST_IP, &mreq.imr_multiaddr);   
    
    // 2. 指定本地网卡的 IP 地址（作为发送数据的源地址）
    inet_aton(LOCAL_IP, &mreq.imr_address);         
    
    // 3. 根据网卡名字（如 ens33）获取它的“身份证号”（索引），确保数据走对物理线路
    mreq.imr_ifindex = if_nametoindex(NETCARD_NAME); 

    // 4. 正式“拍板”：把上面填好的网卡信息告诉套接字(内核)，指定它为组播数据的唯一出口
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) == -1){
        // 如果网卡名字写错或 IP 不匹配，这里会报错
        perror("setsockopt() 设定组播出口失败");
        return -1;
    }


    // [3] 往多播组中发送消息
    group_addr.sin_family = AF_INET;               // 将group_addr结构体变量的sin_family成员设置为AF_INET，表示使用IPv4协议
    inet_aton(MULTICAST_IP, &group_addr.sin_addr); // 将多播组IP地址转换为二进制格式，并赋值给group_addr的sin_addr成员
    group_addr.sin_port = htons(RECV_PORT);        // 将接收端的端口号转换为网络字节序，并赋值给group_addr的sin_port成员
    while(1){
        memset(buf, 0, sizeof(buf)); // 每次发送消息之前，先清空缓冲区，避免上次发送的消息残留在缓冲区中，影响本次发送的消息的正确性
        ssize_t readlen = read(fp, buf, sizeof(buf)); // 从文件中读取数据，参数包括文件描述符fp，缓冲区buf的地址，以及缓冲区的大小
        // 如果读取文件失败
        if(readlen == -1){
            perror("read()");
            return -1;
        }
        // 如果读取到文件末尾
        if(readlen == 0){
            break;
        }
        // 往多播组中发送消息
        sendto(sockfd, buf, readlen, 0, (struct sockaddr *)&group_addr, sizeof(group_addr));
        sleep(1);
    }

    // [4] 关闭套接字
    close(sockfd);
     
    return 0;
}