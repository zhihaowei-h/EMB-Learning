#include "loghawk.h"
#include <sys/wait.h>

#define PROCESS_POOL_SIZE 4 // 进程池数量设置

void worker_process(int worker_id, struct shm_buffer *shm, int semid, int msgid) {
    struct msgbuf message;
    message.mtype = 1;

    while (1) {
        sem_p(semid);
        if (strlen(shm->data) > 0) {
            // 增加时间戳、提取关键字等格式化处理
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            time_str[strlen(time_str)-1] = '\0';

            // 修改后代码
            snprintf(message.mtext, MSG_MAX, "timestamp=%s level=INFO service=order data=%.800s", time_str, shm->data);
            
            // 发送到消息队列
            msgsnd(msgid, &message, strlen(message.mtext) + 1, 0);
            
            memset(shm->data, 0, SHM_SIZE); // 清空处理过的数据
        }
        sem_v(semid);
        usleep(100000);
    }
}

void processor_start() {
    int shmid = shmget(SHM_KEY, sizeof(struct shm_buffer), 0666);
    struct shm_buffer *shm = (struct shm_buffer *)shmat(shmid, NULL, 0);
    int semid = semget(SEM_KEY, 1, 0666);
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    // 多进程并发处理
    for (int i = 0; i < PROCESS_POOL_SIZE; i++) {
        if (fork() == 0) {
            worker_process(i, shm, semid, msgid);
            exit(0);
        }
    }

    // 主进程管理，子进程工作
    while (wait(NULL) > 0); 
}