// 1.3.5 IPC通信模块需求
#include "ipc_modules.h"

// 全局的 IPC ID
static int g_shm_id = -1;
static int g_sem_id = -1;
static int g_msg_id = -1;

// 共享内存映射到当前进程的指针
static SensorDataCache *g_shm_ptr = NULL;

// System V 信号量必须的共用体 (有些系统头文件不自带，需要手动定义)
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    // union semun is defined by including <sys/sem.h>
#else
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};
#endif

// 初始化 IPC 资源
int init_ipc_modules() {
    // 1. 生成统一的 System V IPC Key
    key_t key = ftok(IPC_KEY_PATH, IPC_KEY_PROJ);
    if (key == -1) {
        LOG_FATAL("ftok failed: %s", strerror(errno));
        return -1;
    }

    // 2. 创建/获取共享内存
    g_shm_id = shmget(key, sizeof(SensorDataCache), IPC_CREAT | 0666);
    if (g_shm_id == -1) {
        LOG_FATAL("shmget failed: %s", strerror(errno));
        return -1;
    }

    // 将共享内存映射到当前进程地址空间
    g_shm_ptr = (SensorDataCache *)shmat(g_shm_id, NULL, 0);
    if (g_shm_ptr == (void *)-1) {
        LOG_FATAL("shmat failed: %s", strerror(errno));
        return -1;
    }

    // 3. 创建/获取信号量数组 (只需要 1 个信号量来当互斥锁)
    g_sem_id = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (g_sem_id >= 0) {
        // 创建成功，说明我是第一个，需要初始化信号量的值为 1 (可用状态)
        union semun arg;
        arg.val = 1;
        if (semctl(g_sem_id, 0, SETVAL, arg) == -1) {
            LOG_FATAL("semctl SETVAL failed: %s", strerror(errno));
            return -1;
        }
        // 初始化共享内存内容
        memset(g_shm_ptr, 0, sizeof(SensorDataCache));
        LOG_INFO("System V IPC resources created and initialized.");
    } else if (errno == EEXIST) {
        // 已经存在，直接获取
        g_sem_id = semget(key, 1, 0666);
        LOG_INFO("System V IPC resources attached to existing.");
    } else {
        LOG_FATAL("semget failed: %s", strerror(errno));
        return -1;
    }

    // 4. 创建/获取消息队列
    g_msg_id = msgget(key, IPC_CREAT | 0666);
    if (g_msg_id == -1) {
        LOG_FATAL("msgget failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

// 封装 P 操作 (加锁/减1)
int shm_lock() {
    struct sembuf sb = {0, -1, SEM_UNDO};
    if (semop(g_sem_id, &sb, 1) == -1) {
        if (errno != EINTR) LOG_ERROR("shm_lock failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}

// 封装 V 操作 (解锁/加1)
int shm_unlock() {
    struct sembuf sb = {0, 1, SEM_UNDO};
    if (semop(g_sem_id, &sb, 1) == -1) {
        LOG_ERROR("shm_unlock failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}

// 更新传感器数据 (带锁)
void update_sensor_data_cache(const char *data) {
    if (shm_lock() == 0) {
        g_shm_ptr->total_packets_received++;
        strncpy(g_shm_ptr->latest_sensor_data, data, sizeof(g_shm_ptr->latest_sensor_data) - 1);
        g_shm_ptr->last_update_time = time(NULL);
        shm_unlock();
    }
}

// 读取传感器数据 (带锁)
void read_sensor_data_cache(SensorDataCache *out_cache) {
    if (shm_lock() == 0) {
        memcpy(out_cache, g_shm_ptr, sizeof(SensorDataCache));
        shm_unlock();
    }
}

// 发送异步消息
int send_async_msg(long type, const char *msg) {
    GatewayMessage q_msg;
    q_msg.mtype = type;
    strncpy(q_msg.mtext, msg, sizeof(q_msg.mtext) - 1);
    
    // IPC_NOWAIT 表示队列满了不阻塞，直接报错
    if (msgsnd(g_msg_id, &q_msg, sizeof(q_msg.mtext), IPC_NOWAIT) == -1) {
        LOG_ERROR("msgsnd failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}

// 销毁所有 IPC 资源
void destroy_ipc_modules() {
    if (g_shm_ptr != NULL && g_shm_ptr != (void *)-1) {
        shmdt(g_shm_ptr); // 解除映射
    }
    // 删除共享内存、信号量、消息队列 (严格来说，如果是多进程架构，应该由父进程统一回收)
    shmctl(g_shm_id, IPC_RMID, NULL);
    semctl(g_sem_id, 0, IPC_RMID);
    msgctl(g_msg_id, IPC_RMID, NULL);
    LOG_INFO("System V IPC resources destroyed.");
}