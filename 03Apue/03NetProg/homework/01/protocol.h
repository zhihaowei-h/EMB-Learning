# pragma once

#include <stdint.h>
#define MSGSIZE 128

// [1] 需要一个多播组的地址(224.0.0.0 ~ 239.255.255.255都可以)
#define MULTICAST_IP "238.1.2.3"
// [2] 在创建多播组的时候，需要本地的IP地址(写"0.0.0.0"的好处是将来换主机照样可以使用)
#define LOCAL_IP "0.0.0.0"
// [3] 可以通过ifconfig命令查询当前系统的网卡名称
#define NETCARD_NAME "ens33"
// [4] 接收端的端口
#define RECV_PORT 12345
