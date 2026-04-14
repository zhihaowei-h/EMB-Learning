#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>


// 判断是否不是目录文件
static int is_not_dir(struct stat *fs){
    // 如果 不是目录文件
    if(!S_ISDIR(fs->st_mode)) return -1; // 返回1,表示不是目录文件
    return 0; // 返回0, 表示是目录文件
}

int main(int argc, char *argv[]){
    struct stat fs;//用来存储获取到的文件元信息
    DIR *dp = NULL;//dp指针指向打开的目录流
    struct dirent *entry = NULL;//entry指针指向目录项结构
    int ret = 0;//用来存储错误码
    
    // 如果命令行参数的个数是否少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);
        ret = -1;//存储-1错误码
        goto ERR_1;//由于命令行参数的个数少于2个,跳转到ERR_1
    }
    
    // 如果 获取文件的元信息是否失败
    if(stat(argv[1], &fs) == -1){
        perror("stat()");
        ret = -2;
        goto ERR_1;
    }
    
    // 如果 不是目录文件
    if(is_not_dir(&fs) == -1){
        printf("%s Not A Directory!\n", argv[1]);
        ret = -3;
        goto ERR_1;
    }
    
    dp = opendir(argv[1]);//打开argv[1]目录文件
    // 如果 打开目录文件失败
    if(dp == NULL){
        perror("opendir()");
        ret = -4;
        goto ERR_1;
    }
    
    // 循环读取目录项结构
    while(1){
        errno = 0; // 为了防止errno存储之前的错误码,进行清空处理
        entry = readdir(dp); // 读取目录表中的目录项结构
        // 如果读取结束或者读取失败
        if(entry == NULL){
            // 如果 读取失败或者读取结束
            if(errno != 0){
                perror("readdir()");
                ret = -5;
                goto ERR_2;
            }
            break;
        }
        printf("%ld-%s\n", entry->d_ino, entry->d_name);//打印子目录的inode号以及文件名
    }
ERR_2:
    closedir(dp);//关闭目录流
ERR_1:
    return ret;
}
