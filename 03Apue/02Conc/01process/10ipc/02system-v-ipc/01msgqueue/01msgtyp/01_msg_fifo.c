#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/**
 * 在 System V 消息队列中，发送和接收的数据必须是一个特定的结构体。它没有严格的标准名字，但第一个成员必须是 long 类型的 mtype（消息类型），后面可以/
 * 跟任意数据。我们通常会定义一个结构体来表示这个消息格式，结构体的第一个成员必须是 long 类型的 mtype，这个成员用来标识消息的类型，可以根据这个类型来/
 * 区分不同的消息。第二个成员通常是一个字符数组，用来存储每条消息的内容，大小由我们自己定义，这里我们定义为100字节。
 */

// 消息结构体
struct msgbuf {
    long mtype;      // 消息类型
    char mtext[100]; // 消息正文
};

int main() {
    int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);  // 创建一个私有消息队列
    // 如果创建消息队列失败
    if (msgid == -1) {
        perror("msgget 失败");
        exit(1);
    }

    struct msgbuf msg; // 定义一个消息结构体变量，用于发送和接收消息

    // 依次放入 Type=3, Type=1, Type=2
    msg.mtype = 3; strcpy(msg.mtext, "Type 3 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    msg.mtype = 1; strcpy(msg.mtext, "Type 1 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    msg.mtype = 2; strcpy(msg.mtext, "Type 2 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);

    // 核心操作：msgtyp 传 0
    printf("策略: msgtyp = 0 (读取排在最前面的)\n");
    msgrcv(msgid, &msg, sizeof(msg.mtext), 0, 0);
    
    printf("结果: 取出了 Type=%ld, 内容: %s\n", msg.mtype, msg.mtext);
    // 预期结果：取出了 Type=3（因为它是第一个进去的）

    msgctl(msgid, IPC_RMID, NULL); // 销毁队列
    return 0;
}