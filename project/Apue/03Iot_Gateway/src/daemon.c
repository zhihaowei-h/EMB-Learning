/*===============================================
 *   文件名称：daemon.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：守护进程化模块
 *            实现标准守护进程化流程、PID文件管理
 ================================================*/

#include "../inc/iot_gateway.h"

/*
 * 功能：守护进程化
 * 参数：无
 * 返回：成功返回0，失败返回-1
 */
int daemonize(void)
{
    pid_t pid;
    int i;
    
    // 第一次fork，脱离控制终端
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid > 0) {
        // 父进程退出
        exit(0);
    }
    
    // 子进程继续
    // 创建新会话，成为会话首进程
    if (setsid() < 0) {
        perror("setsid");
        return -1;
    }
    
    // 第二次fork，确保不会重新获得控制终端
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid > 0) {
        // 父进程退出
        exit(0);
    }
    
    // 改变当前工作目录到根目录
    if (chdir("/") < 0) {
        perror("chdir");
        return -1;
    }
    
    // 设置文件创建掩码
    umask(0);
    
    // 关闭所有打开的文件描述符
    for (i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
        close(i);
    }
    
    // 重定向标准输入、输出、错误到/dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }
    
    return 0;
}

/*
 * 功能：创建PID文件并加锁
 * 参数：无
 * 返回：成功返回0，失败返回-1
 */
int create_pid_file(void)
{
    int fd;
    char pid_buf[32];
    struct flock fl;
    
    // 打开PID文件
    fd = open(PID_FILE, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open pid file");
        return -1;
    }
    
    // 设置文件锁
    fl.l_type = F_WRLCK;      // 写锁
    fl.l_whence = SEEK_SET;   // 从文件开始位置
    fl.l_start = 0;           // 偏移量0
    fl.l_len = 0;             // 锁定整个文件
    
    // 尝试获取锁
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "[ERROR] Another instance is already running\n");
        } else {
            perror("fcntl");
        }
        close(fd);
        return -1;
    }
    
    // 清空文件内容
    if (ftruncate(fd, 0) < 0) {
        perror("ftruncate");
        close(fd);
        return -1;
    }
    
    // 写入当前进程PID
    snprintf(pid_buf, sizeof(pid_buf), "%d\n", getpid());
    if (write(fd, pid_buf, strlen(pid_buf)) < 0) {
        perror("write pid");
        close(fd);
        return -1;
    }
    
    // 不关闭文件描述符，保持锁定状态
    // 进程退出时会自动释放锁
    
    return 0;
}

/*
 * 功能：删除PID文件
 * 参数：无
 * 返回：无
 */
void remove_pid_file(void)
{
    unlink(PID_FILE);
}
