#ifndef IPC_MODULES_H
#define IPC_MODULES_H

#include "iot_gateway.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/stat.h>

// IPC 键值生成路径和项目ID (通常用当前目录或配置文件路径)
#define IPC_KEY_PATH "/tmp" 
#define IPC_KEY_PROJ 'G'

// --- 共享内存数据结构 (用于缓存最新传感器状态) ---
typedef struct {
    int total_packets_received;
    char latest_sensor_data[1024];
    time_t last_update_time;
} SensorDataCache;

// --- 消息队列数据结构 ---
typedef struct {
    long mtype;          // 消息类型必须大于0
    char mtext[256];     // 消息内容
} GatewayMessage;

// 初始化和销毁
int init_ipc_modules();
void destroy_ipc_modules();

// 信号量加锁/解锁 (保护共享内存)
int shm_lock();
int shm_unlock();

// 业务层读写接口
void update_sensor_data_cache(const char *data);
void read_sensor_data_cache(SensorDataCache *out_cache);

// 异步消息队列接口
int send_async_msg(long type, const char *msg);
int recv_async_msg(long type, GatewayMessage *out_msg);

#endif // IPC_MODULES_H