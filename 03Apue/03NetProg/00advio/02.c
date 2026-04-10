#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int main() {
    char buf[1024];
    int flags = fcntl(STDIN_FILENO, F_GETFL); // 获取当前文件状态标志  // STDIN_FILENO 是标准输入的文件描述符，写0也可以
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK); // 设置标准输入为非阻塞模式

    // 在这个循环中，我们尝试从标准输入读取数据
    while (1) {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf)); // 尝试从标准输入读取数据
        // 如果没有数据可读，read() 会返回 -1，并且 errno 会被设置为 EAGAIN 或 EWOULDBLOCK 或者其他错误码
        if (n == -1) {
            // 如果是假错误 EAGAIN 或 EWOULDBLOCK，说明当前确实是没有数据可读，我们可以选择继续等待或者去做其他事情
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("还没数据呢，我先去干点别的(比如数个数字)...\n");
                sleep(1); 
                continue;
            } else {  // 如果 errno 不是 EAGAIN 或 EWOULDBLOCK，说明发生了真正的错误，我们应该处理这个错误（比如打印错误信息）并退出循环
                perror("read error");
                break;
            }
        } else if (n == 0) { // 如果 read() 返回 0，说明我们读到了EOF，也就是输入流被关闭了(比如用户按了 Ctrl+D)，在这种情况下，我们应该退出循环，因为没有更多数据可读了
            printf("检测到 EOF，写端已关闭，退出循环\n");
            break;
        } else {
            printf("终于读到了数据: %.*s\n", (int)n, buf);
            break;
        }
    }
    return 0;
}