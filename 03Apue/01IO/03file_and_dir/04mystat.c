// 需求: 获取文件的属主
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>

#define BUFSIZE 10

// 获取文件的类型
static char get_file_type(mode_t st_mode){
    char c = 0;// 用来存储解析出来的文件类型的符号

    switch(st_mode & S_IFMT){
        case S_IFREG : c = '-'; break;
        case S_IFDIR : c = 'd'; break;
        case S_IFCHR : c = 'c'; break;
        case S_IFBLK : c = 'b'; break;
        case S_IFSOCK: c = 's'; break;
        case S_IFIFO : c = 'p'; break;
        case S_IFLNK : c = 'l'; break;
        default      : c = '?'; break;
    }

    return c;
}

static char *get_file_permission(mode_t st_mode, char *buf){
    int mask[BUFSIZE - 1] = {S_IRUSR, S_IWUSR, S_IXUSR,
                            S_IRGRP, S_IWGRP, S_IXGRP,
                            S_IROTH, S_IWOTH, S_IXOTH}; // 存储权限的宏值
    char permission[BUFSIZE] = "rwxrwxrwx"; // 满权限
    int i = 0; // 循环变量

    for(i = 0; i < BUFSIZE - 1; i++){
        // 如果当前文件没有该权限
        if(!(st_mode & mask[i])) permission[i] = '-'; // 由于没有该权限，把该位置的符号修改为'-'
    }
    strncpy(buf, permission, BUFSIZE); // 把局部数组的数据转存到形参指向的存储空间

    return buf;
}

static int get_file_nlink(nlink_t st_nlink){
    return st_nlink;
}

static char *get_file_uname(uid_t st_uid){
    struct passwd *p = NULL; // 指针p指向获取到的用户信息

    p = getpwuid(st_uid); // 根据uid获取该用户的用户信息

    // 如果 获取用户信息 失败
    if(p == NULL){
        perror("getpwuid()"); // 打印错误信息
        return NULL; // 由于获取用户信息失败,结束函数,并且返回NULL
    }
    return p->pw_name; // 返回用户信息中的user name
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

    printf("%c", get_file_type(fs.st_mode));//获取文件的类型
    printf("%s", get_file_permission(fs.st_mode, buf));//获取文件的权限
    printf(" %d ", get_file_nlink(fs.st_nlink));//获取文件的硬链接数
    printf("%s\n", get_file_uname(fs.st_uid));//获取文件的所属者

    return 0;
}