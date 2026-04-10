// FIXME
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

static void *thread_func(void *arg){
    int i = 0;

    for(i = 0; i < 5; i++){
        if(i == 10){
            // pthread_exit((void *)0); // 线程退出，返回NULL
            // pthread_exit(NULL);      // 线程退出，返回NULL
            // return NULL;             // 线程退出，返回NULL
        }
        write(1, "!", 1);  // 向标准输出写入一个!
        sleep(1);  // 线程休眠1秒
    }
}


int main(int argc, char *argv[]){
    pthread_t tid; // 存储新创建的线程的ID
    int ret = 0; // 存储函数调用的返回值
    int i = 0;

    ret = pthread_create(&tid, NULL, thread_func, NULL);
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

    return 0;
}