#include <stdio.h>
#include <shadow.h>
#include <string.h>
#define NAMESIZE 32

int main(void){
    char name[NAMESIZE] = {0}; // 用来存储录入的用户名
    /* 
    对于 char pwd[PWDSIZE] = {0};，整个数组的所有字节都被填充为 '\0'
    以下4个完全等价: 
    char pwd[PWDSIZE] = {0};       // 全零
    char pwd[PWDSIZE] = "";        // 全零
    char pwd[PWDSIZE] = {'\0'};    // 全零
    memset(pwd, 0, PWDSIZE);       // 全零
    */


    struct spwd *pwd = NULL; // 存储/etc/shadow中的密码信息

    // 录入用户名
    fgets(name, NAMESIZE, stdin);
    *strchr(name, '\n') = '\0';
    
    // 获取指定用户的密码信息
    pwd = getspnam(name);
    if(pwd == NULL){
        fprintf(stderr, "获取密码信息失败\n");
    }

    printf("用户%s 的密码原文是: %s\n", name, pwd->sp_pwdp);

    return 0;
}
