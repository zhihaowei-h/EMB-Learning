// 创建4个子进程,实现mycat功能，用信号量数组
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define BLOCK_SIZE 4096  // 每次抢占 4KB 的数据块

union semun { int val; };

// 信号量 P/V 操作封装
void sem_p(int semid) {
    struct sembuf p = {0, -1, SEM_UNDO};
    semop(semid, &p, 1);
}

void sem_v(int semid) {
    struct sembuf v = {0, 1, SEM_UNDO};
    semop(semid, &v, 1);
}

void do_work(int id, int semid, long *shm_ptr, const char *fname) {
    // 每个子进程独立打开文件，拥有独立的“箭头（文件句柄）”
    int fd = open(fname, O_RDONLY);
    if (fd < 0) { perror("open"); exit(1); }

    char buf[BLOCK_SIZE];
    long my_offset;
    struct stat st;
    fstat(fd, &st);

    while (1) {
        // 1. 进入临界区抢占任务块
        sem_p(semid);
        my_offset = *shm_ptr; // 获取当前进度
        if (my_offset >= st.st_size) {
            sem_v(semid);
            break; // 文件读完了
        }
        *shm_ptr += BLOCK_SIZE; // 更新进度供下一个进程抢占
        sem_v(semid);

        // 2. 退出临界区，并行读取
        lseek(fd, my_offset, SEEK_SET); // 移动自己的独立箭头
        ssize_t n = read(fd, buf, BLOCK_SIZE);
        if (n > 0) {
            // 注意：多进程往 stdout 写，输出顺序可能是乱的，块与块之间会交织
            write(STDOUT_FILENO, buf, n);
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { printf("Usage: %s <file>\n", argv[0]); exit(1); }

    // [1] 创建共享内存存进度
    int shmid = shmget(IPC_PRIVATE, sizeof(long), IPC_CREAT | 0666);
    long *shm_ptr = shmat(shmid, NULL, 0);
    *shm_ptr = 0;

    // [2] 创建信号量互斥锁
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    union semun sem_init;
    sem_init.val = 1;
    semctl(semid, 0, SETVAL, sem_init);

    // [3] fork 4个打工人
    for (int i = 0; i < 4; i++) {
        if (fork() == 0) {
            do_work(i, semid, shm_ptr, argv[1]);
            exit(0);
        }
    }

    // [4] 父进程收尾
    for (int i = 0; i < 4; i++) wait(NULL);
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}