#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
    long mtype;
    char mtext[100];
};

int main() {
    int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT); // 创建一个私有消息队列
    if (msgid == -1) {
        perror("msgget 失败");
        exit(1);
    }
    struct msgbuf msg;

    // 依次放入 Type=3, Type=1, Type=2
    msg.mtype = 3; strcpy(msg.mtext, "Type 3 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    msg.mtype = 1; strcpy(msg.mtext, "Type 1 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    msg.mtype = 2; strcpy(msg.mtext, "Type 2 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);

    // 核心操作：msgtyp 传 -3
    printf("策略: msgtyp = -3 (寻找类型 <= 3 中，类型值最小的消息)\n");
    msgrcv(msgid, &msg, sizeof(msg.mtext), -3, 0);
    
    printf("结果: 取出了 Type=%ld, 内容: %s\n", msg.mtype, msg.mtext);
    // 预期分析：满足 <= 3 的有 3, 1, 2。其中 1 最小。
    // 预期结果：取出了 Type=1

    msgctl(msgid, IPC_RMID, NULL); // 销毁队列
    return 0;
}