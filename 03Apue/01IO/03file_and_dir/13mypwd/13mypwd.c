/*
密码的校验过程
[1]输入登录的用户名->fgets(3)注意:由于是标准输入所以有行缓冲，会输入'\n'
[2]输入密码->getpass(3)
[3]读真正的密码->getspnam(3)[4]将输入的密码进行加密->crypt(3)
[5]对比密码->strcmp(3)
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <shadow.h>
#include <crypt.h>

#define NAMESIZE 32

int main(void)
{
    char name[NAMESIZE] = {0};//用来存储录入的用户名
    char *pwd = NULL;//pwd指针指向输入密码的首地址
    struct spwd *tp = NULL;//tp指针指向从/etc/shadow文件中读取的信息
    char *cp = NULL;//cp指针指向输入的密码加密以后的密码字符串

    //[1]输入登录的用户名->fgets(3)
    printf("请输入用户名 : ");//打印提示信息
    fgets(name, NAMESIZE, stdin);//从标准输入流中读取用户名
    *strchr(name, '\n') = '\0';//把name数组中'\n'替换为'\0'

    //[2]输入密码->getpass(3)
    pwd = getpass("请输入密码 : ");//输入密码
    if(pwd == NULL)//判断获取密码是否失败
    {
        perror("getpass()");//打印错误信息
        return -1;//由于输入密码失败,结束程序,并且返回-1
    }

    //[3]读真正的密码->getspnam(3)
    tp = getspnam(name);//根据录入的用户名从/etc/shadow文件中获取该用户的信息
    if(tp == NULL)//判断获取用户信息是否失败
    {
        fprintf(stderr, "获取shadow文件中的用户信息失败!\n");//打印错误信息
        return -2;//由于获取用户信息失败,结束程序,并且返回-2
    }

    //[4]将输入的密码进行加密->crypt(3)
    cp = crypt(pwd, tp->sp_pwdp);//把输入的密码根据加密算法和盐值进行加密
    if(cp == NULL)//判断加密是否失败
    {
        perror("crypt()");//打印错误信息
        return -3;//由于加密失败,结束程序,并且返回-3
    }

    //[5]对比密码->strcmp(3)
    if(!strcmp(tp->sp_pwdp, cp))//判断密码是否相同
        printf("恭喜!登陆成功!\n");
    else
        printf("对不起,密码错误!\n");

    return 0;
}
