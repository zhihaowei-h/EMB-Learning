#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


//[1]把server端的IP地址定义为宏
#define SERVER_IP   "10.11.0.134"
//[2]把server端的PORT定义为宏
#define SERVER_PORT 9527
//[3]通信的最大值定义为宏
#define MSGSIZE     128

#endif