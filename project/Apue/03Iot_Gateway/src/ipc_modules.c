/*===============================================
 *   文件名称：ipc_modules.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：IPC通信模块
 *            实现消息队列、共享内存、信号量等IPC机制
 ================================================*/

#include "../inc/iot_gateway.h"

// 全局IPC资源ID
int g_msgid = -1;
int g_shmid = -1;
int g_semid = -1;
shm_data_t *g_shm_ptr = NULL;

/*
 * 功能：初始化消息队列
 * 参数：无
 * 返回：成功返回0，失败返回-1
 */
static int init_message_queue(void)
{
    key_t key;
    
    // 生成消息队列key
    key = ftok(SHM_KEY_PATH, MSG_KEY_ID);
    if (key < 0) {
        perror("ftok for msgq");
        return -1;
    }
    
    // 创建消息队列
    g_msgid = msgget(key, IPC_CREAT | 0666);
    if (g_msgid < 0) {
        perror("msgget");
        return -1;
    }
    
    log_write(LOG_INFO, "Message queue initialized: msgid=%d", g_msgid);
    
    return 0;
}

/*
 * 功能：初始化共享内存
 * 参数：无
 * 返回：成功返回0，失败返回-1
 */
static int init_shared_memory(void)
{
    key_t key;
    
    // 生成共享内存key
    key = ftok(SHM_KEY_PATH, SHM_KEY_ID);
    if (key < 0) {
        perror("ftok for shm");
        return -1;
    }
    
    // 创建共享内存
    g_shmid = shmget(key, sizeof(shm_data_t), IPC_CREAT | 0666);
    if (g_shmid < 0) {
        perror("shmget");
        return -1;
    }
    
    // 映射共享内存
    g_shm_ptr = (shm_data_t *)shmat(g_shmid, NULL, 0);
    if (g_shm_ptr == (void *)-1) {
        perror("shmat");
        return -1;
    }
    
    // 初始化共享内存数据
    memset(g_shm_ptr, 0, sizeof(shm_data_t));
    g_shm_ptr->sensor_count = 0;
    g_shm_ptr->last_update = time(NULL);
    
    log_write(LOG_INFO, "Shared memory initialized: shmid=%d, size=%zu", 
              g_shmid, sizeof(shm_data_t));
    
    return 0;
}

/*
 * 功能：初始化信号量数组
 * 参数：无
 * 返回：成功返回0，失败返回-1
 */
static int init_semaphore(void)
{
    key_t key;
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    
    // 生成信号量key
    key = ftok(SHM_KEY_PATH, SEM_KEY_ID);
    if (key < 0) {
        perror("ftok for sem");
        return -1;
    }
    
    // 创建信号量数组（1个信号量，用于共享内存互斥访问）
    g_semid = semget(key, 1, IPC_CREAT | 0666);
    if (g_semid < 0) {
        perror("semget");
        return -1;
    }
    
    // 初始化信号量值为1（二值信号量，用作互斥锁）
    arg.val = 1;
    if (semctl(g_semid, 0, SETVAL, arg) < 0) {
        perror("semctl");
        return -1;
    }
    
    log_write(LOG_INFO, "Semaphore initialized: semid=%d", g_semid);
    
    return 0;
}

/*
 * 功能：P操作（申请资源，-1）
 * 参数：semid - 信号量ID
 *       index - 信号量索引
 * 返回：成功返回0，失败返回-1
 */
int sem_p(int semid, int index)
{
    struct sembuf sb;
    
    sb.sem_num = index;
    sb.sem_op = -1;  // P操作
    sb.sem_flg = SEM_UNDO;  // 进程退出时自动释放
    
    if (semop(semid, &sb, 1) < 0) {
        perror("semop P");
        return -1;
    }
    
    return 0;
}

/*
 * 功能：V操作（释放资源，+1）
 * 参数：semid - 信号量ID
 *       index - 信号量索引
 * 返回：成功返回0，失败返回-1
 */
int sem_v(int semid, int index)
{
    struct sembuf sb;
    
    sb.sem_num = index;
    sb.sem_op = 1;   // V操作
    sb.sem_flg = SEM_UNDO;
    
    if (semop(semid, &sb, 1) < 0) {
        perror("semop V");
        return -1;
    }
    
    return 0;
}

/*
 * 功能：初始化所有IPC资源
 * 参数：无
 * 返回：成功返回0，失败返回-1
 */
int ipc_init(void)
{
    // 初始化消息队列
    if (init_message_queue() < 0) {
        return -1;
    }
    
    // 初始化共享内存
    if (init_shared_memory() < 0) {
        return -1;
    }
    
    // 初始化信号量
    if (init_semaphore() < 0) {
        return -1;
    }
    
    log_write(LOG_INFO, "All IPC resources initialized");
    
    return 0;
}

/*
 * 功能：清理所有IPC资源
 * 参数：无
 * 返回：无
 */
void ipc_cleanup(void)
{
    // 解除共享内存映射
    if (g_shm_ptr != NULL && g_shm_ptr != (void *)-1) {
        shmdt(g_shm_ptr);
        g_shm_ptr = NULL;
    }
    
    // 删除共享内存
    if (g_shmid >= 0) {
        shmctl(g_shmid, IPC_RMID, NULL);
        g_shmid = -1;
    }
    
    // 删除消息队列
    if (g_msgid >= 0) {
        msgctl(g_msgid, IPC_RMID, NULL);
        g_msgid = -1;
    }
    
    // 删除信号量
    if (g_semid >= 0) {
        semctl(g_semid, 0, IPC_RMID);
        g_semid = -1;
    }
    
    log_write(LOG_INFO, "All IPC resources cleaned up");
}
