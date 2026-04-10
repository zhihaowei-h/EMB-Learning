#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int shm_id = 0; // 存储共享内存的ID
    int ret = 0; // 存储错误码
    pid_t pid; // 存储子进程的ID
    void *ptr = NULL; // 指向映射成功的虚拟地址空间
    
    // [1] 创建共享内存 -> shmget(2)
    shm_id = shmget(IPC_PRIVATE, 1024, IPC_CREAT | IPC_EXCL | 0600); // 创建共享内存，大小为1024字节，权限为0600（所有者可读写），如果共享内存已存在则返回错误
    // 如果 创建共享内存失败(可能是因为共享内存已存在)，则判断错误码是否为EEXIST，如果是EEXIST，说明共享内存已经存在，可以尝试获取它的ID；如果不是EEXIST，说明发生了其他错误，打印错误信息并返回-1
    if(shm_id == -1){
        // 如果共享内存是否已存在
        if(errno == EEXIST){
            shm_id = shmget(IPC_PRIVATE, 1024, 0); // 获取已存在共享内存的标识
        }
        else{ // 如果共享内存创建失败，并且错误不是EEXIST，说明发生了其他错误，打印错误信息并返回-1
            perror("shmget()"); // 打印错误信息
            ret = -1; // 存储-1错误码
            goto ERR_1; // 由于创建共享内存失败,跳转到ERR_1的位置
        }
    }
    
    // [2] 创建子进程->fork(2)
    pid = fork();//创建子进程
    // 如果 创建子进程失败
    if(pid == -1){
        perror("fork()"); // 打印错误信息
        ret = -2; // 存储-2错误码
        goto ERR_2; // 由于创建子进程失败，跳转到ERR_2的位置
    }
    
    // [3] 父子进程分别将共享内存映射到虚拟地址空间->shmat(2)
    
    //子进程的操作
    if(pid == 0){
        ptr = shmat(shm_id, NULL, 0); // 子进程将共享内存映射到自己的虚拟地址空间，返回映射成功的虚拟地址空间的指针
        // 如果 子进程映射失败
        if(ptr == (void *)-1){
            perror("shmat()"); // 打印错误信息
            exit(1); // 由于子进程映射失败，终止子进程
        }
        memcpy(ptr, "Hello World!", 12); // 往共享内存中写入"Hello World!" // FIXME: memcpy的使用
        shmdt(ptr); // 子进程解除映射关系
        exit(0); // 终止子进程
    }
    
    // 父进程的操作
    wait(NULL);// 等待子进程结束
    ptr = shmat(shm_id, NULL, 0); // 父进程将共享内存映射到自己的虚拟地址空间
    // 如果父进程映射失败
    if(ptr == (void *)-1){
        perror("shmat()"); // 打印错误信息
        ret = -3; // 存储-3错误码
        goto ERR_2; // 由于父进程映射失败，跳转到ERR_2的位置
    }
    puts(ptr); // 把共享内存中的数据打印输出到标准输出中  // FIXME: puts
    shmdt(ptr); // 父进程解除映射关系

    // 以下两行可写也可不写，因为下面有
    // shmctl(shm_id, IPC_RMID, NULL); // 销毁共享内存，避免资源泄漏
    // return 0;                       // 父进程正常退出

ERR_2 :
    shmctl(shm_id, IPC_RMID, NULL); // 销毁共享内存
ERR_1 :
    return ret;
}
