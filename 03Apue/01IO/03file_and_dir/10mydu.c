// 需求: 实现mydu的命令
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#define BUFSIZE 1024

static long _mydu_blocks(const char *pathname)//获取指定文件的所占磁盘空间的块数
{
    struct stat fs;//fs变量用来存储获取到的文件元信息
    long count = 0;//count变量用来存储目录整体的块数
    DIR *dp = NULL;//dp指针指向打开目录的目录流
    struct dirent *entry = NULL;//entry指针指向目录项结构
    char buf[BUFSIZE] = {0};//buf数组中存储的是路径字符串

    if(stat(pathname, &fs) == -1)//判断获取文件元信息是否失败
    {
        perror("stat()");//打印错误信息
        return -1;//由于获取文件元信息失败,结束函数,并且返回-1
    }

    if(!S_ISDIR(fs.st_mode))//判断是否不是目录文件
    {
        return fs.st_blocks;//直接返回非目录文件的块数
    }

    count = fs.st_blocks;//获取目录本身的块数
    dp = opendir(pathname);//打开操作的目录
    if(dp == NULL)//判断打开操作的目录是否失败
    {
        perror("opendir()");//打印错误信息
        return -2;//由于打开操作的目录失败,结束函数,并且返回-2
    }

    while(1)//循环读取目录项结构
    {
        errno = 0;//把errno清空
        entry = readdir(dp);//读取目录流指针指向的目录项结构
        if(entry == NULL)//判断读取目录项结构是否失败或者读取结束
        {
            if(errno != 0)//判断读取目录项结构是否失败
            {
                closedir(dp);//关闭目录流
                perror("readdir()");//打印错误信息
                return -3;//由于读取目录项结构失败,结束函数,并且返回-3
            }
            break;//读取目录项完毕,跳出死循环
        }
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;//跳过目录中的 . 和 .. 目录
        memset(buf, 0, BUFSIZE);//清空buf空间
        strcpy(buf, pathname);//先把当前操作的目录拷贝到buf空间中
        strcat(buf, "/");//拼接目录下的符号
        strcat(buf, entry->d_name);//拼接子文件名
        count += _mydu_blocks(buf);//递归累加子文件的块数
    }
    return count;
}

// 把获取到的所占磁盘空间大小进行单位转换
static long mydu(const char *pathname){
    return _mydu_blocks(pathname) / 2; //所占磁盘空间的块 / 2 = Kb
}

int main(int argc, char *argv[]){
    // 判断命令行参数是否少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);//打印使用说明
        return -1;//由于命令行参数少于2个,结束程序,并且返回-1
    }

    printf("%ldK\t%s\n", mydu(argv[1]), argv[1]);
    //打印argv[1]文件所占磁盘空间的大小

    return 0;
}