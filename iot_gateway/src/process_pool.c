// 1.3.3 线程池模块需求
// src/process_pool.c
#include "process_pool.h"
#include <sys/wait.h>
#include <sys/stat.h>

static int g_process_count = 0;
static pid_t *g_pids = NULL;
static int g_fifo_fd = -1;

// 处理 SIGCHLD 信号，自动收割死去的子进程 (防止变成僵尸进程)
static void sigchld_handler(int signo) {
    (void)signo;
    int saved_errno = errno; // 保存原 errno，防止在信号处理上下文中被篡改
    pid_t pid;
    int status;
    
    // WNOHANG 表示非阻塞，循环收割所有同时退出的子进程
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            LOG_WARN("Plugin Worker (PID: %d) exited normally with code %d", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            LOG_FATAL("Plugin Worker (PID: %d) CRASHED by signal %d!", pid, WTERMSIG(status));
        }
        // 在生产环境中，可以在这里补充重启子进程的逻辑
    }
    errno = saved_errno;
}

// 子进程(插件工人)的核心逻辑
static void plugin_worker(int worker_id) {
    // 忽略 Ctrl+C，防止用户按中断时子进程和父进程一起退出，退出由父进程统一管理
    signal(SIGINT, SIG_IGN); 
    
    // 以只读模式打开 FIFO。如果没有人以写模式打开它，这里会阻塞
    int fd = open(PLUGIN_FIFO_PATH, O_RDONLY);
    if (fd < 0) exit(1);

    LOG_INFO("Plugin Worker %d (PID: %d) isolated and listening on FIFO...", worker_id, getpid());

    char buf[256];
    while (1) {
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            LOG_INFO("[Worker %d] Processing untrusted task: %s", worker_id, buf);
            
            // 模拟一个恶意的崩溃指令
            if (strstr(buf, "CRASH") != NULL) {
                LOG_FATAL("[Worker %d] Oh no! Executing bad code...", worker_id);
                int *p = NULL; 
                *p = 1; // 强行往 0 地址写数据，制造 Segmentation Fault
            }
        } else if (n == 0) {
            // 所有写端都关闭了，稍微休眠防空转
            sleep(1); 
        }
    }
    close(fd);
    exit(0);
}

int init_process_pool(int count) {
    g_process_count = count;
    g_pids = malloc(sizeof(pid_t) * count);

    // 1. 注册 SIGCHLD 信号处理函数
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // 重启被中断的系统调用
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        LOG_ERROR("Failed to setup SIGCHLD handler");
        return -1;
    }

    // 2. 创建命名管道 FIFO
    unlink(PLUGIN_FIFO_PATH); // 先删掉旧的
    if (mkfifo(PLUGIN_FIFO_PATH, 0666) < 0) {
        LOG_ERROR("mkfifo failed: %s", strerror(errno));
        return -1;
    }

    // 3. 循环 Fork 子进程
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // --- 子进程空间 ---
            plugin_worker(i); 
            // 子进程永远不会走到这里，它在 worker 里直接 exit 了
        } else if (pid > 0) {
            // --- 父进程空间 ---
            g_pids[i] = pid;
        } else {
            LOG_ERROR("Fork failed");
            return -1;
        }
    }

    // 4. 父进程以读写模式打开 FIFO (小技巧：RDWR保证不会因为没有读端而阻塞)
    g_fifo_fd = open(PLUGIN_FIFO_PATH, O_RDWR | O_NONBLOCK);
    LOG_INFO("Process pool initialized with %d workers.", count);
    return 0;
}

int send_to_plugin_fifo(const char *cmd) {
    if (g_fifo_fd < 0) return -1;
    // 写入命令到管道，内核会自动调度一个空闲的子进程去读
    write(g_fifo_fd, cmd, strlen(cmd));
    return 0;
}

void destroy_process_pool() {
    // 挨个给子进程发终止信号
    for (int i = 0; i < g_process_count; i++) {
        if (g_pids[i] > 0) kill(g_pids[i], SIGTERM);
    }
    free(g_pids);
    if (g_fifo_fd >= 0) close(g_fifo_fd);
    unlink(PLUGIN_FIFO_PATH);
    LOG_INFO("Process pool and FIFO destroyed.");
}