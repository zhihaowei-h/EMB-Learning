#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

#define BUFSIZE 10

// 获取文件的属组的信息
static char *get_file_gname(gid_t st_gid){
    struct group *p = NULL; // 指针p指向获取到的组信息

    p = getgrgid(st_gid); // 根据gid获取该组的组信息
    // 如果获取组信息是否失败
    if(p == NULL){
        perror("getgrgid()");//打印错误信息
        return NULL;//由于获取组信息失败,结束函数,并且返回NULL
    }
    return p->gr_name;//返回组信息中的group name
}

int main(int argc, char *argv[]){
    struct stat fs;//用来存储获取到的文件元信息
    char buf[BUFSIZE] = {0};//用来存储文件权限的符号

    // 如果命令行参数的个数是否少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);//打印使用说明
        return -1;//由于命令行参数的个数少于2个,结束程序,并且返回-1
    }
    // 如果获取文件的元信息是否失败
    if(stat(argv[1], &fs) == -1){
        perror("stat()"); // 打印错误信息
        return -2; // 由于获取文件的元信息失败,结束程序,并且返回-2
    }

    printf(" %s \n", get_file_gname(fs.st_gid)); // 获取文件的所属组的组名

    return 0;
}
