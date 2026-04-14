#include <stdio.h>

int main(int argc, char *argv[], char *env[]){
    int i = 0; // 循环变量

    printf("argc = %d\n", argc);
    for(i = 0; i < argc; i++) puts(argv[i]);

    for(i = 0; env[i] != NULL; i++) puts(env[i]); // 直接使用main函数的第三个参数env来访问环境变量

    return 0;
}