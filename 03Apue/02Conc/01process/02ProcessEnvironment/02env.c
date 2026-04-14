#include <stdio.h>

extern char **environ; // 声明环境变量数组

int main(int argc, char *argv[]){
    int i = 0; // 循环变量
    
    printf("argc = %d\n", argc);
    for(i = 0; i < argc; i++) puts(argv[i]);
    for(i = 0; environ[i] != NULL; i++) puts(environ[i]);
    
    return 0;
}