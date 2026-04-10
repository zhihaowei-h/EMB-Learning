// 演示fcntl(2)的功能5: 获得/设置记录锁(cmd = F_GETLK、F_SETLK或F_SETLKW)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    // 多个进程打开同一个文件
    int fd = open("shared_log.txt", O_RDWR | O_CREAT, 0644);
    // 如果打开文件失败
    if (fd == -1) { 
        perror("open"); 
        exit(1); 
    }

    pid_t pid = getpid(); // 获取当前进程的PID，方便我们在输出中区分不同的进程
    printf("进程 [%d] 已启动，准备获取文件写锁...\n", pid);

    // 配置一把覆盖整个shared_log.txt文件的写锁 (排他锁)
    struct flock fl;       // 定义一个flock结构体变量，用于描述我们要设置的锁
    fl.l_type = F_WRLCK;   // 设置锁类型为写锁 (排他锁)，表示我们要独占访问这个文件
    fl.l_whence = SEEK_SET;// 锁相对于文件开头的位置，这里我们锁定整个文件，所以从文件开头开始
    fl.l_start = 0;        // 锁从文件开头开始
    fl.l_len = 0;          // 锁的长度为0，表示锁定到文件末尾

    // 尝试阻塞加锁 (W代表Wait，阻塞等待)
    printf("进程 [%d] 正在调用 fcntl(F_SETLKW) 加锁...\n", pid);
    // 如果有其他进程已经往fd上加了锁，那么这个调用会被阻塞，直到锁被释放
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        perror("加锁失败"); 
        exit(1);
    }

    // ====== 临界区开始 ======
    printf(">>> 进程 [%d] 成功给fd加锁！现在独占该文件。\n", pid);
    printf(">>> 进程 [%d] 假装在写入大量数据，持有锁 10 秒钟...\n", pid);
    
    // 在这 10 秒内，如果有其他进程试图给fd加锁，会被死死卡住
    for(int i = 10; i > 0; i--) {
        printf("进程 [%d] 还要持有锁 %d 秒...\n", pid, i);
        sleep(1);
    }

    // ====== 临界区结束 ======

    // 释放锁
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("解锁失败"); exit(1);
    }
    printf("进程 [%d] 已经释放文件锁。其他等待的进程可以进来了！\n", pid);

    close(fd);
    return 0;
}