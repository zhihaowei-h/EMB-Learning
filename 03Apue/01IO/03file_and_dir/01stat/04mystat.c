// 需求: 获取文件的属主
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>

#define BUFSIZE 10

// 获取文件的属主的name
static char *get_file_uname(uid_t st_uid){
    struct passwd *p = NULL; // 指针p指向获取到的用户信息

    p = getpwuid(st_uid); // 根据uid获取该用户的用户信息

    // 如果 获取用户信息 失败
    if(p == NULL){
        perror("getpwuid()"); // 打印错误信息
        return NULL; // 由于获取用户信息失败,结束函数,并且返回NULL
    }
    return p->pw_name; // 返回用户信息中的name
}

int main(int argc, char *argv[]){
    struct stat fs; // 用来存储获取到的文件元信息
    char buf[BUFSIZE] = {0}; // 用来存储文件权限的符号

    // 如果 命令行参数的个数少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]); // 打印使用说明
        return -1; // 由于命令行参数的个数少于2个,结束程序,并且返回-1
    }

    // 如果 获取文件的元信息 失败
    if(stat(argv[1], &fs) == -1){
        perror("stat()");// 打印错误信息
        return -2;// 由于获取文件的元信息失败,结束程序,并且返回-2
    }

    printf("%s\n", get_file_uname(fs.st_uid));//获取文件的所属者的name

    return 0;
}