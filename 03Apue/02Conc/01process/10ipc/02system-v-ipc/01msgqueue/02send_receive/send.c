#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "protocol.h"

int main(int argc, char *argv[]) {
    key_t key;  // 消息队列的键值，这个键值是通过ftok函数生成的，基于指定的文件路径和项目标识符
    int msg_id; // 消息队列的标识符，这个标识符是通过msgget函数获取的，用于后续的消息发送和接收操作
    struct msg_st send_buf; // 用于存储要发送的消息，这个结构体的格式在protocol.h中定义，包含消息类型和消息内容两个成员变量
    int res; // 存储msgsnd函数的返回值

    // [1] 检查命令行参数，要求至少提供消息类型和消息内容两个参数
    if (argc < 3) {
        fprintf(stderr, "用法提示: %s <id> <msg>\n", argv[0]);
        return -1;
    }

    // [2] 生成消息队列的键值
    key = ftok(PATH, PROJ_ID);
    // 如果生成键值失败
    if (key == -1) {
        perror("ftok()");
        return -2;
    }

    // [3] 创建消息队列
    msg_id = msgget(key, IPC_CREAT | IPC_EXCL | 0600); // 创建一个键值为key的消息队列，权限为0600（所有者可读写），如果键值为key的消息队列已存在则返回错误
    // 如果创建失败，可能是因为消息队列已经存在
    if (msg_id == -1) {
        if (errno == EEXIST) {
            // 如果已存在，则直接获取现有标识符
            msg_id = msgget(key, 0);
        } else {  // 创建失败的其他原因
            perror("msgget()");
            return -3;
        }
    }

    // [4] 准备发送消息数据
    memset(&send_buf, 0, sizeof(send_buf)); // 清零 发送缓冲区，确保消息内容干净，避免之前的数据残留影响当前消息的内容
    
    // 解析命令行输入
    send_buf.mtype = atoi(argv[1]); 
    strncpy(send_buf.str, argv[2], STRSIZE - 1);

    // [5] 发送消息
    res = msgsnd(msg_id, &send_buf, strlen(send_buf.str), 0); // 发送消息到消息队列，参数分别是消息队列标识符、消息内容的指针、消息内容的长度和发送选项，这里选项为0表示默认行为（阻塞等待直到消息被成功发送）
    // 如果发送失败
    if (res == -1) {
        perror("msgsnd()");
        return -4;
    }

    printf("成功向队列 %d 发送消息 (Type: %ld)\n", msg_id, send_buf.mtype);

    return 0;
}