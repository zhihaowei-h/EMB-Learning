/*
 * main.c —— LogHawk 入口
 *
 * 职责：
 *   1. 创建所有 IPC 资源（共享内存、信号量、消息队列）
 *   2. 初始化共享内存
 *   3. fork 出 Collector、Processor、Outputer 三个子进程
 *   4. 监控三个子进程，任意一个挂掉则重启
 *   5. 收到 Ctrl+C 后通知所有子进程优雅退出，清理 IPC 资源
 *
 * 使用方式：
 *   编译：make
 *   运行：./loghawk          （前台运行，Ctrl+C 退出）
 *   运行：./loghawk --daemon  （后台守护进程模式）
 *   停止：kill $(cat /tmp/loghawk.pid)
 *   查看：tail -f output_logs/info.log
 */

#include "../inc/loghawk.h"

/* 声明各模块的主函数（定义在各自的 .c 文件里） */
int collector_run(void);
int processor_run(void);
int outputer_run(int as_daemon);

/* ============================================================
 * 全局 IPC 资源 ID（主进程持有，退出时负责清理）
 * ============================================================ */
static int g_shmid = -1;
static int g_semid = -1;
static int g_msgid = -1;

/* 各子进程的 PID */
static pid_t pid_collector  = -1;
static pid_t pid_processor  = -1;
static pid_t pid_outputer   = -1;

static volatile int g_running = 1;

/* ============================================================
 * 清理所有 IPC 资源
 * ============================================================ */
static void cleanup(void) {
    log_info("[Main] 清理 IPC 资源...");
    if (g_shmid >= 0) ipc_shm_destroy(g_shmid);
    if (g_semid >= 0) ipc_sem_destroy(g_semid);
    if (g_msgid >= 0) ipc_msg_destroy(g_msgid);
    unlink(OFFSET_FILE);
    unlink(PID_FILE);
    log_info("[Main] 清理完成，程序退出");
}

/* ============================================================
 * 信号处理：收到 SIGTERM/SIGINT 停止所有子进程
 * ============================================================ */
static void handle_stop(int sig) {
    (void)sig;
    g_running = 0;
    log_info("[Main] 收到退出信号，通知所有子进程...");

    if (pid_collector > 0)  kill(pid_collector,  SIGTERM);
    if (pid_processor > 0)  kill(pid_processor,  SIGTERM);
    if (pid_outputer  > 0)  kill(pid_outputer,   SIGTERM);
}

/* ============================================================
 * 启动子进程的通用函数
 * func_name 用于日志，run_fn 是子进程要执行的函数
 * ============================================================ */
static pid_t start_child(const char *name, int (*run_fn)(void)) {
    pid_t pid = fork();
    if (pid < 0) {
        log_info("[Main] fork %s 失败：%s", name, strerror(errno));
        return -1;
    }
    if (pid == 0) {
        /* 子进程：执行对应模块 */
        exit(run_fn());
    }
    log_info("[Main] 启动 %s，PID=%d", name, pid);
    return pid;
}

/* Outputer 需要一个 int 参数，单独封装一下 */
static int outputer_run_nodaemon(void) {
    return outputer_run(0);   /* 0 = 不用再 daemonize，已经是子进程了 */
}

/* ============================================================
 * 打印启动横幅
 * ============================================================ */
static void print_banner(void) {
    printf("\n");
    printf("  ██╗      ██████╗  ██████╗ ██╗  ██╗ █████╗ ██╗    ██╗██╗  ██╗\n");
    printf("  ██║     ██╔═══██╗██╔════╝ ██║  ██║██╔══██╗██║    ██║██║ ██╔╝\n");
    printf("  ██║     ██║   ██║██║  ███╗███████║███████║██║ █╗ ██║█████╔╝ \n");
    printf("  ██║     ██║   ██║██║   ██║██╔══██║██╔══██║██║███╗██║██╔═██╗ \n");
    printf("  ███████╗╚██████╔╝╚██████╔╝██║  ██║██║  ██║╚███╔███╔╝██║  ██╗\n");
    printf("  ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝\n");
    printf("\n");
    printf("  高性能分布式日志采集与处理系统\n");
    printf("  监控文件：%s\n", SOURCE_LOG);
    printf("  输出目录：output_logs/\n");
    printf("  按 Ctrl+C 停止\n\n");
}

/* ============================================================
 * 主函数
 * ============================================================ */
int main(int argc, char *argv[]) {
    /* 确保输出目录存在 */
    mkdir("output_logs", 0755);

    print_banner();
    log_info("[Main] LogHawk 启动");

    /* ----------------------------------------------------------
     * 第一步：创建 IPC 资源
     * ---------------------------------------------------------- */
    log_info("[Main] 创建共享内存（%zu bytes）...", sizeof(struct shm_ring));
    g_shmid = ipc_shm_create();

    /* 初始化共享内存的环形缓冲区 */
    struct shm_ring *ring = ipc_shm_attach(g_shmid);
    memset(ring, 0, sizeof(struct shm_ring));
    ring->write_idx = 0;
    ring->read_idx  = 0;
    ipc_shm_detach(ring);
    log_info("[Main] 共享内存初始化完成，shmid=%d", g_shmid);

    log_info("[Main] 创建信号量...");
    g_semid = ipc_sem_create();
    log_info("[Main] 信号量创建完成，semid=%d", g_semid);

    log_info("[Main] 创建消息队列...");
    g_msgid = ipc_msg_create();
    log_info("[Main] 消息队列创建完成，msgid=%d", g_msgid);

    /* ----------------------------------------------------------
     * 第二步：注册信号，准备优雅退出
     * ---------------------------------------------------------- */
    signal(SIGTERM, handle_stop);
    signal(SIGINT,  handle_stop);

    /* ----------------------------------------------------------
     * 第三步：启动三个子进程
     * ---------------------------------------------------------- */
    pid_collector = start_child("Collector", collector_run);
    sleep(1);   /* 等 Collector 先就绪 */

    pid_processor = start_child("Processor", processor_run);
    sleep(1);   /* 等 Processor 先就绪 */

    pid_outputer  = start_child("Outputer",  outputer_run_nodaemon);

    printf("所有模块已启动，日志输出到 output_logs/ 目录\n");
    printf("运行日志：output_logs/loghawk.log\n\n");

    /* ----------------------------------------------------------
     * 第四步：主进程监控循环
     * 任意子进程退出时，打印原因并按需重启
     * ---------------------------------------------------------- */
    while (g_running) {
        int status;
        pid_t dead = waitpid(-1, &status, WNOHANG);

        if (dead > 0) {
            const char *who =
                (dead == pid_collector) ? "Collector" :
                (dead == pid_processor) ? "Processor" :
                (dead == pid_outputer)  ? "Outputer"  : "Unknown";

            log_info("[Main] %s (PID=%d) 退出，code=%d",
                     who, dead, WEXITSTATUS(status));

            if (!g_running) break;   /* 正在关闭中，不重启 */

            /* 重启挂掉的子进程 */
            sleep(2);
            if (dead == pid_collector)
                pid_collector = start_child("Collector", collector_run);
            else if (dead == pid_processor)
                pid_processor = start_child("Processor", processor_run);
            else if (dead == pid_outputer)
                pid_outputer  = start_child("Outputer",  outputer_run_nodaemon);
        }

        sleep(1);
    }

    /* ----------------------------------------------------------
     * 第五步：等待所有子进程退出，然后清理
     * ---------------------------------------------------------- */
    log_info("[Main] 等待所有子进程退出...");
    sleep(2);   /* 给子进程时间处理 SIGTERM */

    if (pid_collector > 0) waitpid(pid_collector, NULL, WNOHANG);
    if (pid_processor > 0) waitpid(pid_processor, NULL, WNOHANG);
    if (pid_outputer  > 0) waitpid(pid_outputer,  NULL, WNOHANG);

    cleanup();
    printf("\nLogHawk 已停止。\n");
    return 0;
}
