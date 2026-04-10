#include "loghawk.h"

struct token_bucket bucket = {10, 10, 10}; // 初始令牌，CPS，最大容量

// setitimer + SIGALRM 实现高精度令牌生成
void alarm_handler(int sig) {
    if (bucket.token < bucket.burst) {
        bucket.token++;
    }
}

// daemon() 守护进程化
void make_daemon() {
    if (fork() != 0) exit(0);
    setsid();
    if (fork() != 0) exit(0);
    chdir("/");
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

// 文件锁防止多进程冲突
void enforce_single_instance() {
    int fd = open("/var/run/loghawk.pid", O_RDWR | O_CREAT, 0640);
    struct flock fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        exit(EXIT_FAILURE); // 已有实例运行
    }
}

void outputer_start() {
    make_daemon(); // 守护进程后台运行，脱离终端
    enforce_single_instance(); 

    // 注册定时器信号
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa, NULL);

    struct itimerval timer = {{0, 100000}, {0, 100000}}; // 每100ms补充一个令牌
    setitimer(ITIMER_REAL, &timer, NULL);

    int msgid = msgget(MSG_KEY, 0666);
    struct msgbuf message;
    
    // 打开远端系统或本地落地文件
    int out_fd = open("/tmp/loghawk_final.log", O_WRONLY | O_CREAT | O_APPEND, 0666);

    while (1) {
        if (bucket.token > 0) { // 令牌桶限流保护下游
            if (msgrcv(msgid, &message, MSG_MAX, 1, 0) > 0) { // 从消息队列消费处理后日志
                bucket.token--;
                write(out_fd, message.mtext, strlen(message.mtext));
                write(out_fd, "\n", 1);
            }
        } else {
            pause(); // 等待令牌
        }
    }
}