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

int main(void){
    char name[NAMESIZE] = {0}; // 存储录入的用户名
    char *pwd = NULL; // 存储输入的密码原文
    struct spwd *tp = NULL; // tp指针指向从/etc/shadow文件中的密码信息
    char *cp = NULL; // cp指针指向输入的密码加密以后的加密密码

    // [1] 输入登录的用户名->fgets(3)
    printf("请输入用户名: "); // 打印提示信息
    fgets(name, NAMESIZE, stdin); // 从标准输入中读取用户名
    *strchr(name, '\n') = '\0'; // 把name数组中'\n'替换为'\0'

    // [2] 输入密码->getpass(3)
    pwd = getpass("请输入密码 : "); // 输入密码
    // 如果获取密码是否失败
    if(pwd == NULL){
        perror("getpass()");
        return -1;
    }

    // [3] 读真正的密码->getspnam(3)
    tp = getspnam(name); // 根据录入的用户名从/etc/shadow文件中获取该用户的密码信息
    // 如果 获取用户信息是否失败
    if(tp == NULL){
        fprintf(stderr, "获取shadow文件中的用户信息失败!\n");
        return -2;
    }

    //[4]将输入的密码进行加密->crypt(3)
    cp = crypt(pwd, tp->sp_pwdp); // 把输入的密码根据加密算法和盐值进行加密
    // 如果 加密是否失败
    if(cp == NULL){
        perror("crypt()");//打印错误信息
        return -3;//由于加密失败,结束程序,并且返回-3
    }

    //[5]对比密码->strcmp(3)
    // 如果密码相同
    if(!strcmp(tp->sp_pwdp, cp)) printf("恭喜!登陆成功!\n");
    else printf("对不起,密码错误!\n");

    return 0;
}
