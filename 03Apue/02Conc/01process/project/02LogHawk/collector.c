#include "loghawk.h"

#define TARGET_LOG "/var/log/syslog"

void collector_start() {
    printf("[INFO] 日志采集器启动成功，监听文件: %s\n", TARGET_LOG); // 模拟采集器启动
    
    // 初始化共享内存与信号量
    int shmid = shmget(SHM_KEY, sizeof(struct shm_buffer), IPC_CREAT | 0666);
    struct shm_buffer *shm = (struct shm_buffer *)shmat(shmid, NULL, 0);
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    // 记录采集偏移量结构体 [cite: 101]
    struct log_offset offset_record;
    strncpy(offset_record.filename, TARGET_LOG, sizeof(offset_record.filename));
    
    int fd = open(TARGET_LOG, O_RDONLY); // 打开日志文件 [cite: 63]
    if (fd < 0) {
        perror("打开日志文件失败");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    stat(TARGET_LOG, &st); // 获取文件大小与 inode [cite: 59]
    offset_record.inode = st.st_ino;
    offset_record.offset = lseek(fd, 0, SEEK_END); // lseek 定位到保存的偏移量 [cite: 64]

    char buffer[1024];
    while (1) { // 循环监控 [cite: 68]
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1); // 读取新增数据 [cite: 65]
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            sem_p(semid);
            // 写入共享内存 [cite: 66] (此处为简化示意，真实环境需处理环形写指针逻辑)
            strncpy(shm->data, buffer, SHM_SIZE); 
            shm->write_pos += bytes_read;
            sem_v(semid);
            
            offset_record.offset += bytes_read; // 更新偏移量 [cite: 67]
        } else {
            usleep(500000); // 轮询检测文件变化 [cite: 60]
        }
    }
}