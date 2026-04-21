/*
 * ipc.c —— IPC 资源管理
 * 封装共享内存、信号量、消息队列的创建/连接/销毁
 * 其他模块直接调用这里的函数，不用关心系统调用细节
 */

#include "../inc/loghawk.h"

/* ============================================================
 * 共享内存
 * ============================================================ */

/* 创建共享内存，返回 shmid */
int ipc_shm_create(void) {
    int shmid = shmget(SHM_KEY, sizeof(struct shm_ring), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("[ipc] shmget 失败");
        exit(1);
    }
    return shmid;
}

/* 把共享内存映射到当前进程地址空间，返回指针 */
struct shm_ring *ipc_shm_attach(int shmid) {
    struct shm_ring *ring = shmat(shmid, NULL, 0);
    if (ring == (void *)-1) {
        perror("[ipc] shmat 失败");
        exit(1);
    }
    return ring;
}

/* 解除映射（本进程不再使用，但内存还存在） */
void ipc_shm_detach(struct shm_ring *ring) {
    shmdt(ring);
}

/* 彻底销毁共享内存（释放系统资源） */
void ipc_shm_destroy(int shmid) {
    shmctl(shmid, IPC_RMID, NULL);
}

/* ============================================================
 * 信号量
 * ============================================================ */

/* semctl 需要的联合体 */
union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

/* 创建信号量，初始值为 1（相当于"锁是开着的"） */
int ipc_sem_create(void) {
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("[ipc] semget 失败");
        exit(1);
    }
    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);
    return semid;
}

/* 加锁：P 操作，把信号量减 1；若已为 0 则阻塞等待 */
void ipc_sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

/* 解锁：V 操作，把信号量加 1，唤醒等待的进程 */
void ipc_sem_unlock(int semid) {
    struct sembuf op = {0, +1, 0};
    semop(semid, &op, 1);
}

/* 销毁信号量 */
void ipc_sem_destroy(int semid) {
    semctl(semid, 0, IPC_RMID);
}

/* ============================================================
 * 消息队列
 * ============================================================ */

/* 创建消息队列，返回 msgid */
int ipc_msg_create(void) {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("[ipc] msgget 失败");
        exit(1);
    }
    return msgid;
}

/* 销毁消息队列 */
void ipc_msg_destroy(int msgid) {
    msgctl(msgid, IPC_RMID, NULL);
}
