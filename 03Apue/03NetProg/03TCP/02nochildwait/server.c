// 接收端
#include "protocol.h"

int main(void){
    int tcp_socket;              // 存储创建成功的流式套接字描述符
    int new_socket;              // 存储accept(2)返回的描述符
    struct sockaddr_in laddr;    // 存储本地的地址
    pid_t pid;                   // 存储子进程的标识
    struct sigaction act;        // 存储SIGCHLD信号的行为
    act.sa_handler = SIG_DFL;    // 默认行为
    
    act.sa_flags = SA_NOCLDWAIT;    // 加入子进程不会变为僵尸进程的要求
    sigaction(SIGCHLD, &act, NULL); // 为SIGCHLD信号设置新行为

    //[1]创建流式套接字
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0); // 创建流式套接字
    // 如果 创建流式套接字失败
    if(tcp_socket == -1){
        perror("socket()");//打印错误信息
        return -1;//由于创建流式套接字失败,结束程序,并且返回-1
    }

    //[2]绑定地址
    laddr.sin_family = AF_INET;             // 指定IPv4协议
    //inet_aton("0.0.0.0", &laddr.sin_addr);// 转换本地地址(也可以使用宏)
    laddr.sin_addr.s_addr = INADDR_ANY;     // 转换本地地址(INADDR_ANY宏是"0.0.0.0"的二进制形式)
    laddr.sin_port = htons(SERVER_PORT);    // 转换本地端口号
    if(bind(tcp_socket, (struct sockaddr *)&laddr, sizeof(laddr)) == -1){
        perror("bind()");
        close(tcp_socket);
        return -2;
    }

    //[3]套接字处于监听状态
    if(listen(tcp_socket, 20) == -1)//判断监听连接请求是否失败
    {
        perror("listen()");//打印错误信息
        close(tcp_socket);//关闭流式套接字
        return -3;//由于监听连接请求失败,结束程序,并且返回-3
    }

    //[4]接受连接请求
    while(1)
    {
        new_socket = accept(tcp_socket, NULL, NULL);//接受连接请求
        if(new_socket == -1)//判断接收连接请求是否失败
        {
            perror("accept()");//打印错误信息
            close(tcp_socket);//关闭流式套接字
            return -4;//由于接收连接请求失败,结束程序,并且返回-4
        }
        //new_sockfd : 用于数据交互 old_sockfd : 用于接受连接请求
        pid = fork();//创建子进程
        if(pid == -1)//判断创建子进程是否失败
        {
            perror("fork()");//打印错误信息
            close(tcp_socket);//关闭流式套接字
            return -5;//由于创建子进程失败,结束程序,并且返回-5
        }
        //[5]连接成功后,进行I/O操作
        if(pid == 0)//子进程负责数据交互
        {
            write(new_socket, "Hello ZaCk", 10);//写数据
            close(new_socket);//关闭数据交互的套接字
            close(tcp_socket);//关闭流式套接字
            exit(0);//终止子进程,并且设置状态为0
        }
        //父进程关闭新套接字
        close(new_socket);//关闭数据交互的套接字
    }
    //[6]关闭流式套接字
    close(tcp_socket);//关闭流式套接字

    return 0;
}