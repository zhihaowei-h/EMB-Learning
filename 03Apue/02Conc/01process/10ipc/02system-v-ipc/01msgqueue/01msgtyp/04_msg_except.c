// 开启 GNU 扩展以使用 MSG_EXCEPT
#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// 解决冲突：把结构体名字改成 my_msgbuf
struct my_msgbuf {
    long mtype;
    char mtext[100];
};

int main() {
    int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }
    
    struct my_msgbuf msg; // 使用我们自己命名的结构体

    // 依次放入 Type=3, Type=1, Type=2
    msg.mtype = 3; strcpy(msg.mtext, "Type 3 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    msg.mtype = 1; strcpy(msg.mtext, "Type 1 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
    msg.mtype = 2; strcpy(msg.mtext, "Type 2 Data"); msgsnd(msgid, &msg, sizeof(msg.mtext), 0);

    // 核心操作：msgtyp 传 3，并且 flag 加上 MSG_EXCEPT
    printf("策略: msgtyp = 3, flag = MSG_EXCEPT (跳过类型 3，读下一个)\n");
    
    // 注意这里依然要把 msg 的地址传进去
    msgrcv(msgid, &msg, sizeof(msg.mtext), 3, MSG_EXCEPT);
    
    printf("结果: 取出了 Type=%ld, 内容: %s\n", msg.mtype, msg.mtext);
    // 预期结果：排在第一的是 Type=3，被排除；读取下一个，即 Type=1

    msgctl(msgid, IPC_RMID, NULL); // 销毁队列
    return 0;
}