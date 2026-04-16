#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define PWDSIZE 32

int main(void){
    char pwd[PWDSIZE] = {0};
    
    // pwd = getpass("请输入密码: "); // 数组名是指针常量，不能直接被赋值，所以要用strcpy

    strcpy(pwd, getpass("请输入密码: "));

    printf("您输入的密码是: %s\n", pwd);

    return 0;
}
