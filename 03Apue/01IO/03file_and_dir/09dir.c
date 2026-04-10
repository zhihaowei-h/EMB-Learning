// 需求: 读取目录的实验
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char *argv[])
{   
    struct stat fs;//用来存储获取到的文件元信息
    DIR *dp = NULL;//dp指针指向打开的目录流
    struct dirent *entry = NULL;//entry指针指向目录项结构
    int ret = 0;//用来存储错误码
    
    if(argc < 2)//判断命令行参数的个数是否少于2个
    {
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);//打印使用说明
        ret = -1;//存储-1错误码
        goto ERR_1;//由于命令行参数的个数少于2个,跳转到ERR_1
    }
    
    if(stat(argv[1], &fs) == -1)//判断获取文件元信息是否失败
    {
        perror("stat()");//打印错误信息
        ret = -2;//存储-2错误码
        goto ERR_1;//由于获取文件元信息失败,跳转到ERR_1
    }
    
    if(!S_ISDIR(fs.st_mode))//判断是否不是目录文件
    {
        printf("%s Not A Directory!\n", argv[1]);//打印提示信息
        ret = -3;//存储-3错误码
        goto ERR_1;//由于不是目录文件,跳转到ERR_1
    }
    
    dp = opendir(argv[1]);//打开argv[1]目录文件
    if(dp == NULL)//判断打开目录文件是否失败
    {
        perror("opendir()");//打印错误信息
        ret = -4;//存储-4错误码
        goto ERR_1;//由于打开目录文件失败,跳转到ERR_1
    }
    
    while(1)//循环读取目录项结构
    {
        errno = 0;//为了防止errno存储之前的错误码,进行清空处理
        entry = readdir(dp);//读取目录流中的目录项结构
        if(entry == NULL)//判断是否读取结束或者读取失败
        {
            if(errno != 0)//判断是否读取失败
            {
                perror("readdir()");//打印错误信息
                ret = -5;//存储-5错误码
                goto ERR_2;//由于读取目录项结构失败,跳转到ERR_2
            }
            break;//跳出死循环
        }
        printf("%ld-%s\n", entry->d_ino, entry->d_name);//打印子目录的inode号以及文件名
    }
ERR_2:
    closedir(dp);//关闭目录流
ERR_1:
    return ret;
}
