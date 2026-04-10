# pragma once

#include <stdint.h>
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
#include <net/if.h>

#define MSGSIZE 128

// [1] 需要一个多播组的地址(224.0.0.0 ~ 239.255.255.255都可以)
#define MULTICAST_IP "234.2.3.4"
// [2] 在创建多播组的时候，需要本地的IP地址(写"0.0.0.0"的好处是将来换主机照样可以使用)
#define LOCAL_IP "0.0.0.0"
// [3] 可以通过ifconfig命令查询当前系统的网卡名称
#define NETCARD_NAME "ens33"
// [4] 接收端的端口
#define RECV_PORT 9527


struct data_st{
    int8_t id;
    char msg[MSGSIZE];
}__attribute__((paked));  // 单字节对齐c
