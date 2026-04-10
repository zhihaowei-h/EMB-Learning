// 需求: 使用glob(3)读取目录的实验
#include <stdio.h>
#include <glob.h>

int main(int argc, char *argv[])
{   
    glob_t gs;//gs变量用于存储解析到的目录个数以及目录名
    int i = 0;//循环变量
    
    //if(glob("/etc/*", 0, NULL, &gs) != 0)//判断解析目录是否失败
    if(glob("/home/zack/*", GLOB_NOSORT, NULL, &gs) != 0)
    {
        fprintf(stderr, "glob() Is Failed!\n");//打印错误信息
        return -1;//由于解析目录失败,结束程序,并且返回-1
    }
    
    glob("/home/zack/.*", GLOB_APPEND, NULL, &gs);//追加解析隐藏的文件
    
    for(i = 0; i < gs.gl_pathc; i++)//循环遍历目录中子文件名
        printf("%s\n", gs.gl_pathv[i]);
    
    globfree(&gs);//释放glob(3)开辟的空间
    
    return 0;
}