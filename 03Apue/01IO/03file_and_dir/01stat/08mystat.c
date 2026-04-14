//需求: 获取文件的文件名
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUFSIZE 10

int main(int argc, char *argv[]){
    struct stat fs;//用来存储获取到的文件元信息
    char buf[BUFSIZE] = {0};//用来存储文件权限的符号

    // 如果 命令行参数的个数是否少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);
        return -1; // 由于命令行参数的个数少于2个,结束程序,并且返回-1
    }

    // 如果 获取文件的元信息是否失败
    if(stat(argv[1], &fs) == -1){
        perror("stat()");//打印错误信息
        return -2;//由于获取文件的元信息失败,结束程序,并且返回-2
    }

    printf("%s\n", argv[1]);//获取文件的文件名

    return 0;
}