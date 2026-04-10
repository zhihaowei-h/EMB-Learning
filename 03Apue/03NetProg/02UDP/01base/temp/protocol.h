#pragma once 

#define IP "192.168.0.32"
#define PORT 9527
#define MSGSIZE 128
#include <stdint.h>

// 数据包结构体
struct data{
    int8_t data_id;   // 数据报ID
    char str[MSGSIZE];// 数据报中的内容
}__attribute__((packed));