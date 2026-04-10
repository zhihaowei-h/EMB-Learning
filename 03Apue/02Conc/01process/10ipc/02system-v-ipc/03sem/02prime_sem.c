// 创建4个子进程,实现查询100-300中的质数，不用信号量数组
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/shm.h>

/**
 * 定义一个联合体，用于初始化信号量数组
 * 
 * 为什么用共用体（Union）？
 * semctl 不仅能初始化一个信号量（SETVAL），还能一次性初始化整个数组（SETALL），或者获取状态（IPC_STAT）。不同的命令需要不同类型的参数，所以用共用体来节省内存。
 *  SETVAL：只需要一个 int 值（比如初始化为 1）。
 *  SETALL：需要一个 unsigned short 类型的数组指针。
 *  IPC_STAT：需要一个指向 struct semid_ds 结构体的指针。
 * 所以说，完整的 union semun 定义应该包含这三种类型的成员，但在实际使用中，我们通常只需要其中的一种(比如 val)，所以这里只定义了val成员:
 * union semun {
 *     int val;                 // 用于 SETVAL (设置单个信号量的值)
 *     struct semid_ds *buf;    // 用于 IPC_STAT, IPC_SET (获取/设置属性)
 *     unsigned short  *array;  // 用于 GETALL, SETALL (操作整个数组)
 * };
 * 这样，无论你传的是整数、数组地址还是结构体地址，它们在内存中都占用同一块空间。内核只需要根据你传的 cmd，去解析这块空间的“不同侧面”即可。
 **/ 
union semun{
    int val; // 信号量的初始值
};

#define MIN 100 // 最小的质数
#define MAX 300 // 最大的质数

// 全局变量(父子进程共享)
int *shm_ptr = NULL; // 指向共享内存的指针
int semid = 0;       // 信号量数组ID

// 将信号量初始化为1(用作互斥锁)
void sem_init(int semid){
    union semun tmp;
    tmp.val = 1; // 将信号量的初始值设置为1(表明该信号量只能被一个进程占用，其他进程如果申请该信号量时发现它的值为0，则会阻塞等待)
    semctl(semid, 0, SETVAL, tmp); // 将semid信号量数组中下标为0的信号量的初始值设置为1
}

// 功能: 对信号量执行P操作 -> 申请锁(信号量-1，如果信号量的值为0，则阻塞等待)
void sem_p(int semid){
    struct sembuf sem_p = {0, -1, SEM_UNDO}; // 定义一个 信号量操作结构体 变量，表示P操作。行为: 将信号量数组中下标为0的信号量的值减1,如果信号量的值为0，则阻塞等待。SEM_UNDO表示当进程退出时，自动将信号量还原(即加1，释放锁)
    semop(semid, &sem_p, 1);                 // 对信号量数组中下标为0的信号量执行P操作
}

// 功能: 对信号量执行V操作 -> 释放锁(信号量+1，如果有阻塞等待的进程，则唤醒其中一个)
void sem_v(int semid){
    struct sembuf sem_v = {0, 1, SEM_UNDO}; // 定义一个sembuf结构体变量，表示V操作。行为: 将信号量数组中下标为0的信号量的值加1，SEM_UNDO表示当进程退出时，自动将信号量还原(即减1，申请锁)
    semop(semid, &sem_v, 1); // 对信号量数组中下标为0的信号量执行1次V操作
}

// 判断一个整数是否为质数
static int is_prime(int num){
    int i = 0;
    sleep(1); // 模拟耗时操作
    if(num <= 1){
        return 0;
    }
    for(i = 2; i <= num/2; i++){
        if(num % i == 0){
            return 0;
        }
    }
    return 1;
}

// 子进程的工作函数 -> 抢占共享内存中的整数，判断它是否为质数，并将结果打印出来
void work(int n){
    int num = 0; // 用于存储临界区中的整数
    // 子进程循环抢任务
    while(1){
        sem_p(semid); // 申请锁(加锁)
        // 如果共享内存中的整数大于MAX，则说明没有任务了，释放锁并退出循环
        if(*shm_ptr > MAX) { // 如果共享内存中的整数大于MAX，则说明没有任务了，释放锁并退出循环
            sem_v(semid); // 释放锁(解锁) | 干完活后一定要第一时间解锁，否则可能会死锁
            break;
        }
        num = *shm_ptr; // 抢占共享内存中的整数
        (*shm_ptr)++;   // 将共享内存中的整数加1，为下一个子进程抢占做准备
        sem_v(semid);   // 释放锁(解锁)
        // 如果 抢占到的整数是否为质数
        if(is_prime(num)) {
            printf("[%d]: %d\n", n, num); // 如果是质数，则打印出来
        }
    }
}

int main(void) {
    int i = 0, j = 0; // 循环变量
    pid_t pid;        // 子进程的PID
    int n = 0;        // 循环变量，表示子进程的编号(0-3)
    int shmid = 0;    // 共享内存的ID

    // [1] 创建共享内存
    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0666); // 创建一个大小为sizeof(int)的共享内存，权限为0666
    shm_ptr = shmat(shmid, NULL, 0); // 将shmid共享内存映射到当前调用进程的虚拟地址空间，返回映射后的地址(之后操作这个地址就相当于是在操作共享内存了)
    *shm_ptr = MIN; // 将共享内存中存储的整数初始化为MIN

    // [2] 创建信号量数组
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT|0666); // 创建一个包含1个信号量的信号量数组，权限为0666
    sem_init(semid); // 初始化信号量数组，即将信号量数组中所有的信号量的初始值设置为1(用作互斥锁)

    // [3] 创建4个子进程(pid为0-3)，每个子进程执行work函数
    for (n = 0; n < 4; n++) {
        pid = fork();
        // 如果 创建子进程失败
        if (pid < 0) {
            perror("fork()");
            exit(1);
        }
        // 子进程操作
        if(pid == 0){
            work(n); // 执行子进程的工作函数 -> 抢占共享内存中的整数，判断它是否为质数，并将结果打印出来
            exit(0); // 子进程退出
        }
    }

    // 子进程退出
    switch(n){
        case 0:
        case 1:
        case 2:
        case 3: exit(0);
    }

    // 父进程操作
    for(i = 0; i < 4; i++) {
        wait(NULL);
    }

    shmdt(shm_ptr); // 解除共享内存的映射关系
    shmctl(shmid, IPC_RMID, NULL); // 删除共享内存
    semctl(semid, 0, IPC_RMID);    // 删除信号量
    
    return 0;
}