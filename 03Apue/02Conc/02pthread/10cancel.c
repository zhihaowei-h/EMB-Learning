// 线程取消示例
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

static void cancel_handler1(void *arg){
    printf("cancel_handler1: %s\n", __FUNCTION__); // 输出取消处理程序1的消息
}
static void cancel_handler2(void *arg){
    printf("cancel_handler2: %s\n", __FUNCTION__); // 输出取消处理程序2的消息

}

static void *thr_job(void *arg){
    int i = 0;
    pthread_cleanup_push(cancel_handler1, NULL); // 注册取消处理程序1
    pthread_cleanup_push(cancel_handler2, NULL); // 注册取消处理程序2
    for(i = 0; i < 10; i++){
        write(1, "!", 1);  // 向标准输出写入一个!
        sleep(1);          // 线程休眠1秒
    }
    pthread_cleanup_pop(1); // 弹出取消处理程序2，同时执行线程取消处理程序2
    pthread_cleanup_pop(0); // 弹出取消处理程序1，但不执行线程取消处理程序1
    return NULL;            // 线程退出，返回NULL
}


int main(int argc, char *argv[]){
    pthread_t tid; // 存储新创建的线程的ID
    int ret = 0; // 存储函数调用的返回值
    int i = 0;

    ret = pthread_create(&tid, NULL, thr_job, NULL);
    // 如果 线程创建失败
    if (ret != 0) {
        fprintf(stderr, "pthread_create(): %s\n", strerror(ret)); // KILLME
        exit(1); // 由于创建进程失败，退出程序，并返回错误码1
    }

    for(i = 0; i < 3; i++){
        write(1, "*", 1); // 向标准输出写入一个星号
        sleep(1);  // 主线程休眠1秒
    }

    pthread_join(tid, NULL); // 等待上面创建的线程退出，然后回收线程资源
    // pthread_cancel(tid);
    
    return 0;
}