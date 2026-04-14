#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 10

// 获取文件的硬链接数
static int get_file_nlink(nlink_t st_nlink){
    return st_nlink;
}

int main(int argc, char *argv[]){
    struct stat fs; // 用来存储获取到的文件元信息
    char buf[BUFSIZE] = {0};// 用来存储文件权限的符号
    
    // 如果 命令行参数的个数少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);// 打印使用说明
        return -1;
    }
    
    // 如果 获取文件的元信息 失败
    if(stat(argv[1], &fs) == -1){
        perror("stat()"); // 打印错误信息
        return -2; // 由于获取文件的元信息失败,结束程序,并且返回-2
    }

    printf(" %d \n", get_file_nlink(fs.st_nlink));// 获取文件的硬链接数

    return 0;
}