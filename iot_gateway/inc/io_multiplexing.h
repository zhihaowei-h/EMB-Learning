#ifndef IO_MULTIPLEXING_H
#define IO_MULTIPLEXING_H

#include "iot_gateway.h"
#include "thread_pool.h"

// 将文件描述符设置为非阻塞模式
int set_nonblocking(int fd);

// 初始化 TCP 监听服务器
int init_tcp_server(int port);

// 初始化 UDP 服务器
int init_udp_server(int port);

// 启动主事件循环 (新增 sig_pipe_read_fd 参数)
void start_event_loop(ThreadPool *pool, int sig_pipe_read_fd);

#endif // IO_MULTIPLEXING_H