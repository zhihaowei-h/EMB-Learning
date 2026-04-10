#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "protocol.h"

int main(void)
{
    int tcp_socket;//存储创建成功的流式套接字描述符
    struct sockaddr_in raddr;//存储对端的地址
    char msg[MSGSIZE] = {0};//存储对端传输的数据

    //[1]创建流式套接字
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);//创建流式套接字
    if(tcp_socket == -1)//判断创建流式套接字是否失败
    {
        perror("socket()");//打印错误信息
        return -1;//由于创建流式套接字失败,结束程序,并且返回-1
    }

    //[2]和对端请求连接
    raddr.sin_family = AF_INET;//指定IPv4协议
    inet_aton(SERVER_IP, &raddr.sin_addr);//转换对端地址
    raddr.sin_port = htons(SERVER_PORT);//转换对端端口号
    if(connect(tcp_socket, (struct sockaddr *)&raddr, sizeof(raddr)) == -1)
    {
        perror("connect()");//打印错误信息
        close(tcp_socket);//关闭流式套接字
        return -2;//由于和对端请求连接失败,结束程序,并且返回-2
    }

    //[3]一旦请求成功,进行I/O操作
    read(tcp_socket, msg, MSGSIZE);//读取对端发送过来的数据

    puts(msg);//把读取到的数据打印到标准输出中

    //[4]关闭流式套接字
    close(tcp_socket);//关闭流式套接字

    return 0;
}