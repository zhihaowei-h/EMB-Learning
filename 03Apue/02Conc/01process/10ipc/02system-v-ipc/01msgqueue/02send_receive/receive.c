#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include "protocol.h"
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(void){
    key_t key;  // 定义一个key_t类型的变量key，用来存储生成的键值
    int msg_id; // 定义一个int类型的变量msg_id，用来存储消息队列的标识符
    struct msg_st recv_buf; // 定义一个msg_st类型的变量msg，用来存储接收到的消息
    size_t count = 0; // 定义一个size_t类型的变量count，用来统计接收到的消息的字节数

    // [1]通过ftok函数生成一个唯一的键值，参数是之前定义的文件路径和项目标识符
    key = ftok(PATH, PROJ_ID);
    // 如果生成键值失败
    if (key == -1) {
        perror("ftok()");
        return -1;
    }

    // [2] 创建key对应的消息队列，权限为0600（所有者可读写），如果键值为key的消息队列已存在则返回错误
    msg_id = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    // 如果创建消息队列失败
    if (msg_id == -1) {
        // 如果错误是EEXIST，说明消息队列已经存在，可以尝试获取它的标识符
        if(errno == EEXIST) {
            // 如果消息队列已经存在，尝试获取它的标识符
            msg_id = msgget(key, 0);
        } else { // 如果消息队列创建失败，并且错误不是EEXIST，说明发生了其他错误，打印错误信息并返回-2
            perror("msgget()");
            return -2;
        }
    }
    // 以上的代码和将来写的send.c一摸一样

    // [3]等待接收
    while(1){
        memset(&recv_buf, 0, sizeof(recv_buf)); // 每次接收前都要清空接收缓冲区，避免上次接收的消息残留在缓冲区中，影响本次接收的结果
        count = msgrcv(msg_id, &recv_buf, STRSIZE, 0, 0);
        // 如果接收消息失败，打印错误信息并返回-3
        if(count == -1){
            perror("msgrcv()");
            msgctl(msg_id, IPC_RMID, NULL); // 删除消息队列，避免资源泄漏
            return -3;
        }
        printf("Received message: %s\n", recv_buf.str);
    }
    
    return 0;
}