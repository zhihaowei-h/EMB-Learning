#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

#include "fsm.h"

#define TTY1    "/dev/tty9"
#define TTY2    "/dev/tty10"

int main(void){
    fsm_t *fsm12 = NULL; // 有限状态机指针：负责从 tty9 读取并写入 tty10
    fsm_t *fsm21 = NULL; // 有限状态机指针：负责从 tty10 读取并写入 tty9
    int fd1, fd2;        // 用于保存打开的文件描述符

    /* [1] 初始化 poll 结构体数组
     * 我们需要同时监控 fd1 和 fd2 两个描述符，所以定义长度为 2 的数组 */
    struct pollfd pfd[2];

    // 打开第一个终端 /dev/tty9 (默认阻塞模式打开)
    fd1 = open(TTY1, O_RDWR);
    if(fd1 == -1){
        perror("open(TTY1) 失败");
        return -1;
    }
    write(fd1, "[****tty9****]", 14); // 在终端显示标识信息

    // 打开第二个终端 /dev/tty10 (带非阻塞标志打开，确保 FSM 运行顺畅)
    fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
    if(fd2 == -1){
        perror("open(TTY2) 失败");
        close(fd1);
        return -2;
    }
    write(fd2, "[!!!tty10!!!]", 13); // 在终端显示标识信息

    // 分别初始化两个方向的状态机实例
    fsm_init(&fsm12, fd1, fd2);
    fsm_init(&fsm21, fd2, fd1);

    // =======================================

    /* [2] 绑定文件描述符到 poll 结构体
     * 注意：由于 poll 的 events 成员是独立的，我们在此固定绑定 fd，
     * 而具体的监听意愿（events）将在循环内部动态更新。 */
    pfd[0].fd = fd1; 
    pfd[1].fd = fd2;

    /* 推动有限状态机的核心循环：
     * 只要两个状态机都没有进入终止态（STATE_T），循环就继续 */
    while(fsm12->state != STATE_T || fsm21->state != STATE_T) {
        /* [3] 重置监听意愿（清理上次循环残留的 events）
         * 因为每轮循环状态机的 state 可能会变化，我们需要重新计算本轮需要监听什么。 */
        pfd[0].events = 0;
        pfd[1].events = 0;

        /* [4] 根据 fsm12 (tty9 -> tty10) 的当前状态设置监听位
         * - 若处于读态 (STATE_R)，说明需要从 fd1 读取，监听 fd1 的 POLLIN。
         * - 若处于写态 (STATE_W)，说明需要向 fd2 写入，监听 fd2 的 POLLOUT。 */
        if(fsm12->state == STATE_R)
            pfd[0].events |= POLLIN;
        if(fsm12->state == STATE_W)
            pfd[1].events |= POLLOUT;

        /* [5] 根据 fsm21 (tty10 -> tty9) 的当前状态设置监听位
         * - 若处于读态 (STATE_R)，说明需要从 fd2 读取，监听 fd2 的 POLLIN。
         * - 若处于写态 (STATE_W)，说明需要向 fd1 写入，监听 fd1 的 POLLOUT。 */
        if(fsm21->state == STATE_R)
            pfd[1].events |= POLLIN;
        if(fsm21->state == STATE_W)
            pfd[0].events |= POLLOUT;

        /* [6] 调用 poll 系统调用（核心阻塞点）
         * 参数解析：
         * - pfd: 监听数组首地址
         * - 2: 监听的文件描述符个数
         * - -1: 无限期阻塞，直到有任一事件就绪或被信号打断 */
        if(poll(pfd, 2, -1) == -1)
        {
            // 阻塞调用可能被系统信号中断，需检查 errno
            if(errno == EINTR) continue; // 信号中断属于非致命错误，重启循环
            perror("poll() 出错");
            goto ERR_1;
        }

        /* [7] 检查结果并推动 fsm12 
         * 如果 poll 告知 fd1 可读 (POLLIN) 或 fd2 可写 (POLLOUT)，
         * 且该事件正是 fsm12 当前所期待的，则调用 fsm_drive 推进其状态。 */
        if((pfd[0].revents & POLLIN) || (pfd[1].revents & POLLOUT))
            fsm_drive(fsm12);

        /* [8] 检查结果并推动 fsm21
         * 同理，检查 fd2 是否可读或 fd1 是否可写，满足条件则推动 fsm21。 */
        if((pfd[1].revents & POLLIN) || (pfd[0].revents & POLLOUT))
            fsm_drive(fsm21);
    }

ERR_1:
    // 任务结束或出错跳转，清理资源
    fsm_destroy(fsm12);
    fsm_destroy(fsm21);

    close(fd2);
    close(fd1);

    return 0;
}