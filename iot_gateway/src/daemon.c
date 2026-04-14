// 1.3.1 守护进程与信号处理需求
#include "iot_gateway.h"

// 文件锁定函数，返回 0 表示成功加锁，返回 -1 表示失败
int lockfile(int fd) {
    struct flock fl;
    fl.l_type = F_WRLCK;  // 写锁
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;         // 锁定整个文件
    return fcntl(fd, F_SETLK, &fl);
}

// 检查是否已经有实例在运行
int already_running() {
    int fd;
    char buf[16];
    
    // 打开/创建 PID 文件
    fd = open(PID_FILE, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        gateway_log(LOG_ERROR, __FILE__, __LINE__, "Cannot open PID file %s: %s", PID_FILE, strerror(errno));
        exit(1);
    }
    
    // 尝试加锁
    if (lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return 1; // 已经被其他实例锁定
        }
        gateway_log(LOG_ERROR, __FILE__, __LINE__, "Cannot lock PID file %s: %s", PID_FILE, strerror(errno));
        exit(1);
    }
    
    // 写入当前 PID
    ftruncate(fd, 0);
    snprintf(buf, sizeof(buf), "%ld\n", (long)getpid());
    write(fd, buf, strlen(buf));
    
    // 注意：这里不要 close(fd)，否则锁会被释放！进程退出时系统会自动关闭并释放锁。
    return 0;
}


// 守护进程化函数
void daemonize() {
    pid_t pid;
    
    // 1. 清除文件模式创建掩码
    umask(0);
    
    // 2. 第一次 fork
    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(1);
    } else if (pid != 0) {
        exit(0); // 父进程退出，子进程成为孤儿进程被 init/systemd 收养
    }
    
    // 3. 创建新会话，成为脱缰之马
    setsid();
    
    // 4. 第二次 fork (确保子进程不是会话首进程，永远无法打开控制终端)
    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }
    
    // 5. 更改当前工作目录到根目录 (防止占用可卸载的文件系统)
    if (chdir("/") < 0) {
        perror("chdir error");
        exit(1);
    }
    
    // 6. 检查单例运行并写入 PID
    if (already_running()) {
        fprintf(stderr, "Gateway is already running.\n");
        exit(1);
    }
    
// 7. 关闭标准文件描述符，重定向到 /dev/null
    // 先关闭默认的 0, 1, 2
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // 此时 0, 1, 2 是空闲的。open 会自动分配最小可用的 0
    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0); // 复制 0，自动分配最小的 1
    int fd2 = dup(0); // 复制 0，自动分配最小的 2
    
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        // 注意：这里哪怕打印日志，也只能写到文件里了，因为终端屏幕(1,2)已经被关了
        gateway_log(LOG_ERROR, __FILE__, __LINE__, "Unexpected file descriptors");
        exit(1);
    }
}