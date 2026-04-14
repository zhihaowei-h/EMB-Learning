// 需求: 获取文件最后修改内容的时间
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUFSIZE 10
#define TIMELEN 32

// 获取文件的修改时间
static char *get_file_mtime(time_t tm, char *tbuf){
    struct tm *p = NULL; // p指针指向转换后的时间结构体

    p = localtime(&tm); // 把时间戳转换成时间结构体
    // 如果转换失败
    if(p == NULL){
        perror("localtime()");
        return NULL;
    }
    strftime(tbuf, TIMELEN, "%m月 %d %H:%M", p); // 根据时间结构体获取格式化时间的字符串

    return tbuf;
}

int main(int argc, char *argv[]){
    struct stat fs;//用来存储获取到的文件元信息
    char buf[BUFSIZE] = {0};//用来存储文件权限的符号
    char tbuf[TIMELEN] = {0};//用来存储文件最后修改内容时间的字符串

    // 如果命令行参数的个数是否少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);
        return -1;//由于命令行参数的个数少于2个,结束程序,并且返回-1
    }

    // 如果 获取文件的元信息是否失败
    if(stat(argv[1], &fs) == -1){
        perror("stat()");
        return -2;//由于获取文件的元信息失败,结束程序,并且返回-2
    }

    printf("%s\n", get_file_mtime(fs.st_mtime, tbuf));//获取文件的修改时间

    return 0;
}
