/*
 * processor.c —— 日志处理模块
 *
 * 职责：
 *   1. 主进程创建 WORKER_COUNT 个子进程（进程池）
 *   2. 每个子进程从共享内存读取原始日志
 *   3. 解析日志级别（INFO/WARN/ERROR）
 *   4. 通过消息队列发送给 Outputer
 *   5. 子进程异常退出时，主进程自动重启它
 *
 * 与其他模块的关系：
 *   [共享内存] → Processor → [消息队列] → Outputer
 */

#include "../inc/loghawk.h"

/* ============================================================
 * 全局变量（主进程使用）
 * ============================================================ */
static int shmid  = -1;
static int semid  = -1;
static int msgid  = -1;
static struct shm_ring *ring = NULL;

static pid_t worker_pids[WORKER_COUNT];
static volatile int running = 1;

/* ============================================================
 * 信号处理
 * ============================================================ */
static void handle_stop(int sig) {
    (void)sig;
    running = 0;
    /* 通知所有子进程退出 */
    for (int i = 0; i < WORKER_COUNT; i++) {
        if (worker_pids[i] > 0)
            kill(worker_pids[i], SIGTERM);
    }
    log_info("[Processor] 收到停止信号，正在关闭所有工人");
}

/* ============================================================
 * 从共享内存读取一行日志
 * 如果没有数据返回 0，有数据返回 1
 * ============================================================ */
static int ring_read(char *buf) {
    ipc_sem_lock(semid);

    int idx = ring->read_idx;
    if (ring->slots[idx].used == 0) {
        /* 没有数据 */
        ipc_sem_unlock(semid);
        return 0;
    }

    /* 读取数据 */
    strncpy(buf, ring->slots[idx].data, LINE_MAX - 1);
    buf[LINE_MAX - 1] = '\0';

    /* 标记槽为空闲，并移动读指针 */
    ring->slots[idx].used = 0;
    ring->read_idx = (idx + 1) % SHM_SLOTS;

    ipc_sem_unlock(semid);
    return 1;
}

/* ============================================================
 * 工人子进程的工作循环
 * 参数 worker_id：工人编号（0, 1, 2...）
 * ============================================================ */
static void worker_loop(int worker_id) {
    log_info("[Worker-%d] 启动，PID=%d", worker_id, getpid());

    char linebuf[LINE_MAX];
    struct log_msg msg;
    int processed = 0;

    while (1) {
        /* 尝试从共享内存读一行 */
        if (!ring_read(linebuf)) {
            /* 没有数据，等一会儿 */
            usleep(50000);   /* 50ms */
            continue;
        }

        /* 解析日志级别 */
        int level = parse_level(linebuf);

        /* 只处理 INFO/WARN/ERROR，OTHER 丢弃（通常是 kernel 的噪声） */
        if (level == LEVEL_OTHER) continue;

        /* 构造消息 */
        msg.mtype = level;
        snprintf(msg.mtext, MSG_TEXT_MAX, "[Worker-%d] %s", worker_id, linebuf);

        /* 发送到消息队列 */
        if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) < 0) {
            if (errno != EINTR)
                log_info("[Worker-%d] msgsnd 失败：%s", worker_id, strerror(errno));
        } else {
            processed++;
            if (processed % 10 == 0)
                log_info("[Worker-%d] 已处理 %d 条", worker_id, processed);
        }
    }
}

/* ============================================================
 * 启动第 i 号工人进程
 * ============================================================ */
static void start_worker(int i) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("[Processor] fork 失败");
        return;
    }

    if (pid == 0) {
        /* 子进程：连接 IPC 资源后进入工作循环 */
        shmid = shmget(SHM_KEY, sizeof(struct shm_ring), 0666);
        ring  = ipc_shm_attach(shmid);
        semid = semget(SEM_KEY, 1, 0666);
        msgid = msgget(MSG_KEY, 0666);

        worker_loop(i);

        /* 正常情况不会到这里 */
        ipc_shm_detach(ring);
        exit(0);
    } else {
        /* 父进程：记录子进程 PID */
        worker_pids[i] = pid;
        log_info("[Processor] 启动 Worker-%d，PID=%d", i, pid);
    }
}

/* ============================================================
 * 主函数：进程池管理循环
 * ============================================================ */
int processor_run(void) {
    log_info("[Processor] 启动，进程池大小：%d", WORKER_COUNT);

    signal(SIGTERM, handle_stop);
    signal(SIGINT,  handle_stop);

    /* 初始化 */
    memset(worker_pids, 0, sizeof(worker_pids));

    /* 启动所有工人 */
    for (int i = 0; i < WORKER_COUNT; i++) {
        start_worker(i);
    }

    /* 主进程监控循环：检查有没有工人挂掉 */
    while (running) {
        int status;
        pid_t dead = waitpid(-1, &status, WNOHANG);

        if (dead > 0) {
            /* 找出是哪个工人 */
            for (int i = 0; i < WORKER_COUNT; i++) {
                if (worker_pids[i] == dead) {
                    int code = WEXITSTATUS(status);
                    log_info("[Processor] Worker-%d (PID=%d) 退出，code=%d", i, dead, code);

                    if (running) {
                        /* 异常退出，1 秒后重启 */
                        sleep(1);
                        log_info("[Processor] 重启 Worker-%d", i);
                        start_worker(i);
                    }
                    break;
                }
            }
        }
        sleep(1);
    }

    /* 等待所有子进程退出 */
    for (int i = 0; i < WORKER_COUNT; i++) {
        if (worker_pids[i] > 0)
            waitpid(worker_pids[i], NULL, 0);
    }

    log_info("[Processor] 所有工人已退出");
    return 0;
}
