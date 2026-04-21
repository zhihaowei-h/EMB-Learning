/*
 * collector.c —— 日志采集模块
 *
 * 职责：
 *   1. 监控 /var/log/syslog，发现新内容就读出来
 *   2. 支持断点续传（记住上次读到哪里）
 *   3. 检测日志轮转（文件被替换时自动重置）
 *   4. 把读到的每一行写入共享内存环形缓冲区
 *
 * 与其他模块的关系：
 *   Collector → [共享内存] → Processor
 */

#include "../inc/loghawk.h"

/* ============================================================
 * 全局变量
 * ============================================================ */
static int shmid  = -1;
static int semid  = -1;
static struct shm_ring *ring = NULL;
static volatile int running  = 1;

/* ============================================================
 * 信号处理：收到 SIGTERM/SIGINT 时优雅退出
 * ============================================================ */
static void handle_stop(int sig) {
    (void)sig;
    running = 0;
    log_info("[Collector] 收到停止信号，准备退出");
}

/* ============================================================
 * 把一行日志写入共享内存的下一个空闲槽
 * 如果环形缓冲区满了，会等待（阻塞）
 * ============================================================ */
static void ring_write(const char *line) {
    while (1) {
        ipc_sem_lock(semid);   /* 加锁，独占环形缓冲区 */

        int idx = ring->write_idx;
        if (ring->slots[idx].used == 0) {
            /* 这个槽是空的，可以写 */
            strncpy(ring->slots[idx].data, line, LINE_MAX - 1);
            ring->slots[idx].data[LINE_MAX - 1] = '\0';
            ring->slots[idx].used = 1;
            ring->write_idx = (idx + 1) % SHM_SLOTS;  /* 移到下一个槽 */

            ipc_sem_unlock(semid);
            return;
        }

        /* 缓冲区满了，解锁后等一会儿再试 */
        ipc_sem_unlock(semid);
        usleep(10000);   /* 等 10ms */
    }
}

/* ============================================================
 * 主函数：采集循环
 * ============================================================ */
int collector_run(void) {
    log_info("[Collector] 启动，监控文件：%s", SOURCE_LOG);

    /* 注册退出信号 */
    signal(SIGTERM, handle_stop);
    signal(SIGINT,  handle_stop);

    /* 连接共享内存和信号量（由 main 进程创建好了） */
    shmid = shmget(SHM_KEY, sizeof(struct shm_ring), 0666);
    ring  = ipc_shm_attach(shmid);
    semid = semget(SEM_KEY, 1, 0666);

    /* 读取上次的偏移量（断点续传） */
    long offset = offset_load();

    /* 打开日志文件 */
    int fd = open(SOURCE_LOG, O_RDONLY);
    if (fd < 0) {
        log_info("[Collector] 无法打开 %s：%s（需要 sudo 权限？）", SOURCE_LOG, strerror(errno));
        /* 没有权限时，用一个模拟文件做测试 */
        log_info("[Collector] 切换到测试模式，监控 /tmp/test_syslog.log");
        fd = open("/tmp/test_syslog.log", O_RDONLY | O_CREAT, 0644);
        if (fd < 0) {
            perror("open test log");
            return 1;
        }
    }

    /* 如果是第一次运行（offset == -1），跳到文件末尾，不读旧日志 */
    if (offset < 0) {
        offset = lseek(fd, 0, SEEK_END);
        offset_save(offset);
        log_info("[Collector] 首次运行，从末尾开始，当前偏移：%ld", offset);
    } else {
        lseek(fd, offset, SEEK_SET);
        log_info("[Collector] 断点续传，从偏移 %ld 继续", offset);
    }

    /* 记录当前文件的 inode，用于检测日志轮转 */
    struct stat st;
    fstat(fd, &st);
    ino_t current_inode = st.st_ino;

    char linebuf[LINE_MAX];
    char charbuf[1];
    int  line_pos = 0;

    /* ============================================================
     * 主采集循环
     * ============================================================ */
    while (running) {
        /* 检查文件是否发生了轮转（inode 变化 或 文件变小） */
        struct stat new_st;
        if (stat(SOURCE_LOG, &new_st) == 0) {
            if (new_st.st_ino != current_inode) {
                /* 日志轮转了：旧文件被重命名，新建了一个同名文件 */
                log_info("[Collector] 检测到日志轮转（inode 变化），重新打开文件");
                close(fd);
                fd = open(SOURCE_LOG, O_RDONLY);
                if (fd < 0) {
                    sleep(2);
                    continue;
                }
                fstat(fd, &new_st);
                current_inode = new_st.st_ino;
                offset = 0;
                line_pos = 0;
            }
        }

        /* 检查文件是否有新内容 */
        fstat(fd, &st);
        if (st.st_size <= offset) {
            /* 没有新内容，休眠后继续 */
            sleep(1);
            continue;
        }

        /* 逐字符读取，按行分割 */
        int n = read(fd, charbuf, 1);
        if (n <= 0) {
            sleep(1);
            continue;
        }

        offset++;

        if (charbuf[0] == '\n' || line_pos >= LINE_MAX - 1) {
            /* 读到一行结束 */
            if (line_pos > 0) {
                linebuf[line_pos] = '\0';
                ring_write(linebuf);   /* 写入共享内存 */
            }
            line_pos = 0;
        } else {
            linebuf[line_pos++] = charbuf[0];
        }

        /* 每读 100 个字节保存一次偏移量 */
        if (offset % 100 == 0) {
            offset_save(offset);
        }
    }

    /* 退出清理 */
    offset_save(offset);
    close(fd);
    ipc_shm_detach(ring);
    log_info("[Collector] 已退出");
    return 0;
}
