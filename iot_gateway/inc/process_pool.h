#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include "iot_gateway.h"

// 命名管道在系统中的路径
#define PLUGIN_FIFO_PATH "/tmp/iot_plugin_fifo"

// 初始化进程池
int init_process_pool(int worker_count);

// 销毁进程池
void destroy_process_pool();

// 通过 FIFO 向子进程发送指令
int send_to_plugin_fifo(const char *cmd);

#endif // PROCESS_POOL_H