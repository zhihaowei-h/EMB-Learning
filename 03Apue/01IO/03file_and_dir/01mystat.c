// 需求: 获取文件的类型
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// 获取文件的类型
static char get_file_type(mode_t st_mode){
    char c = 0; // 用来存储解析出来的文件类型的符号

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

int main(int argc, char *argv[]){
    struct stat fs;//用来存储获取到的文件元信息

    // 如果 命令行参数的个数少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]); // 打印使用说明
        return -1;
    }

    // 如果 获取文件的元信息 失败
    if(stat(argv[1], &fs) == -1){
        perror("stat()"); // 打印错误信息
        return -2; // 由于获取文件的元信息失败,结束程序,并且返回-2
    }

    printf("%c\n", get_file_type(fs.st_mode)); // 获取文件的类型

    return 0;
}