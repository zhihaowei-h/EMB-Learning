/**
 * @file protocol.h 
 * @brief 
 * @version 0.1
 * @date 2026-04-08
 */
# pragma once
#include <stdint.h>
#define MSGSIZE 128

// [1] 约定地址 = IP + PORT
#define SERVER_IP "192.168.0.32"
#define SERVER_PORT 9527

// [2] 约定双方通信的格式(灵活自由)
struct data_st{
    int8_t id;  // 通信id  // 因为是1个字节，所以不存在字节序问题
    char msg[MSGSIZE];     // 因为是字符串，所以不存在字节序问题(为什么字符串不存在字节序问题，因为字符串本质是多个字符组成的，一个字符就是一个字节)
}__attribute__((packed));