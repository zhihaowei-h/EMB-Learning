/*===============================================
 *   文件名称：process_pool.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：进程池模块
 *            管理子进程、通过管道通信、自动回收僵尸进程
 ================================================*/

#include "../inc/iot_gateway.h"

// 全局进程池
process_pool_t *g_process_pool = NULL;

/*
 * 功能：子进程工作函数
 * 参数：pipe_fd - 与父进程通信的管道
 * 返回：无（子进程会退出）
 */
static void child_worker(int pipe_fd)
{
    char cmd[BUFFER_SIZE];
    ssize_t n;
    
    // 关闭不需要的文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    
    // 子进程循环等待任务
    while (1) {
        // 从管道读取命令
        n = read(pipe_fd, cmd, sizeof(cmd) - 1);
        if (n <= 0) {
            break;  // 管道关闭或错误，退出子进程
        }
        
        cmd[n] = '\0';
        
        // 执行命令（这里简化处理，实际应该执行真正的插件程序）
        if (strcmp(cmd, "exit") == 0) {
            break;
        }
        
        // 模拟处理任务
        sleep(1);
        
        // 向管道写入结果（可选）
        const char *result = "done";
        write(pipe_fd, result, strlen(result));
    }
    
    close(pipe_fd);
    exit(0);
}

/*
 * 功能：创建进程池
 * 参数：process_count - 进程数量
 * 返回：进程池指针，失败返回NULL
 */
process_pool_t *process_pool_create(int process_count)
{
    process_pool_t *pool;
    int i;
    pid_t pid;
    
    if (process_count <= 0) {
        fprintf(stderr, "[ERROR] Invalid process count\n");
        return NULL;
    }
    
    // 分配进程池结构
    pool = (process_pool_t *)malloc(sizeof(process_pool_t));
    if (pool == NULL) {
        perror("malloc");
        return NULL;
    }
    
    pool->process_count = process_count;
    
    // 分配进程数组
    pool->processes = (process_t *)malloc(sizeof(process_t) * process_count);
    if (pool->processes == NULL) {
        perror("malloc");
        free(pool);
        return NULL;
    }
    
    // 创建子进程
    for (i = 0; i < process_count; i++) {
        // 创建管道
        if (pipe(pool->processes[i].pipe_fd) < 0) {
            perror("pipe");
            process_pool_destroy(pool);
            return NULL;
        }
        
        // fork子进程
        pid = fork();
        if (pid < 0) {
            perror("fork");
            process_pool_destroy(pool);
            return NULL;
        }
        
        if (pid == 0) {
            // 子进程
            close(pool->processes[i].pipe_fd[1]);  // 关闭写端
            child_worker(pool->processes[i].pipe_fd[0]);
            // 不会到达这里
        }
        
        // 父进程
        pool->processes[i].pid = pid;
        pool->processes[i].busy = 0;
        close(pool->processes[i].pipe_fd[0]);  // 关闭读端
    }
    
    log_write(LOG_INFO, "Process pool created with %d processes", process_count);
    
    return pool;
}

/*
 * 功能：在进程池中执行命令
 * 参数：pool - 进程池指针
 *       cmd - 要执行的命令
 * 返回：成功返回0，失败返回-1
 */
int process_pool_execute(process_pool_t *pool, const char *cmd)
{
    int i;
    
    if (pool == NULL || cmd == NULL) {
        return -1;
    }
    
    // 查找空闲进程
    for (i = 0; i < pool->process_count; i++) {
        if (!pool->processes[i].busy) {
            // 向子进程发送命令
            if (write(pool->processes[i].pipe_fd[1], cmd, strlen(cmd)) < 0) {
                perror("write to pipe");
                return -1;
            }
            
            pool->processes[i].busy = 1;
            log_write(LOG_DEBUG, "Task assigned to process %d", pool->processes[i].pid);
            return 0;
        }
    }
    
    log_write(LOG_WARN, "No idle process available");
    return -1;
}

/*
 * 功能：销毁进程池
 * 参数：pool - 进程池指针
 * 返回：无
 */
void process_pool_destroy(process_pool_t *pool)
{
    int i;
    
    if (pool == NULL) {
        return;
    }
    
    // 向所有子进程发送退出命令
    for (i = 0; i < pool->process_count; i++) {
        if (pool->processes[i].pid > 0) {
            const char *exit_cmd = "exit";
            write(pool->processes[i].pipe_fd[1], exit_cmd, strlen(exit_cmd));
            close(pool->processes[i].pipe_fd[1]);
        }
    }
    
    // 等待所有子进程退出
    for (i = 0; i < pool->process_count; i++) {
        if (pool->processes[i].pid > 0) {
            waitpid(pool->processes[i].pid, NULL, 0);
        }
    }
    
    // 释放内存
    free(pool->processes);
    free(pool);
    
    log_write(LOG_INFO, "Process pool destroyed");
}
