#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LNAMESIZE   128
#define HNAMESIZE   128
#define PWDSIZE     256
#define BUFSIZE     16

static int get_env_pwd(char *lname, char *hname, char *pwd)
{
    char *tmpname = NULL;//临时存储当前登陆用户的用户名

    //[a]获取当前登陆用户的用户名
    if((tmpname = getenv("LOGNAME")) == NULL)//判断获取当前登陆用户的用户名是否失败
        return -1;//由于获取当前登陆用户的用户名失败,结束函数,并且返回-1
    else
        strcpy(lname, tmpname);//把临时存储的用户名拷贝到lname指向的存储空间

    //[b]获取当前终端的主机名
    if(gethostname(hname, HNAMESIZE) < 0)//判断获取当前终端的主机名是否失败
    {
        perror("gethostname()");//打印错误信息
        return -2;//由于获取当前终端的主机名失败,结束函数,并且返回-2
    }

    //[c]获取当前工作目录的路径
    if(getcwd(pwd, PWDSIZE) == NULL)//判断获取当前工作目录的路径是否失败
    {
        perror("getcwd()");//打印错误信息
        return -3;//由于获取当前工作目录的路径失败,结束函数,并且返回-3
    }

    return 0;//3个信息都回去成功,返回0
}

static int get_cmd_line(char **line, size_t *length)
{
    if(getline(line, length, stdin) == -1)//判断读取一行内容是否失败
    {
        perror("getline()");//打印错误信息
        return -1;//由于读取一行内容失败,结束函数,并且返回-1
    }
    return 0;//成功读取了一行内容,返回0
}

static int parse_string(char *str, char *delim, char **buf, int size)
{
    int i = 0;//循环变量

    while(i < size)//防止越界
    {
        buf[i] = strtok(str, delim);//分割字符串
        if(buf[i] == NULL)//判断分割字符串是否完毕
            break;//跳出循环
        i++;//循环变量自增
        str = NULL;//第一次指向原串,之后再调用需要指向NULL
    }

    return 0;
}

int main(void)
{
    char lname[LNAMESIZE] = {0};//用来存储当前登陆用户的用户名
    char hname[HNAMESIZE] = {0};//用来存储当前终端的主机名
    char pwd[PWDSIZE] = {0};//用来存储当前工作目录的路径
    char *line = NULL;//存储读取一行指令的首地址
    size_t length = 0;//存储读取到的字节数
    char *buf[BUFSIZE] = {0};//用来存储分割出的子串的首地址
    pid_t pid;//用来存储子进程的标识

    while(1)//死循环
    {
        //[1]打印终端提示符 "用户名@主机名:当前工作路径"
        if(get_env_pwd(lname, hname, pwd) < 0)//判断获取终端信息是否失败
            exit(1);//由于获取终端信息失败,终止进程,并且返回状态1
        else
            printf("%s@%s:%s$ ", lname, hname, pwd);//打印终端提示符(可扩展)

        //[2]等待读一行输入,例如 : "ls -l\n"
        if(get_cmd_line(&line, &length) < 0)//判断读取一行指令是否失败
            exit(2);//由于读取一行指令失败,终止进程,并且返回状态2

        //[3]分割字符串,需要把"ls -l\n",分割出"ls" "-l" "\n"
        parse_string(line, " \n", buf, BUFSIZE);//分割命令 选项 参数

        //[4]创建子进程
        pid = fork();//创建子进程
        if(pid < 0)//判断创建子进程是否失败
        {
            perror("fork()");//打印错误信息
            exit(3);//由于创建子进程失败,终止进程,并且返回状态3
        }

        //[5]替换子进程
        if(pid == 0)//子进程的操作
        {
            execvp(buf[0], buf);//替换子进程
            perror("execvp()");//打印错误信息
            exit(4);//由于替换子进程失败,终止进程,并且返回状态4
        }
        //[6]父进程等待子进程结束
        else//父进程的操作
            wait(NULL);
    }

    free(line);//释放空间

    return 0;
}
