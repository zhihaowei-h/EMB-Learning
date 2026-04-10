#include <stdio.h>

extern char **environ;

int main(int argc, char *argv[])
{   
    int i = 0;//循环变量
    
    printf("argc = %d\n", argc);
    
    for(i = 0; i < argc; i++)
        puts(argv[i]);
    printf("\n");
    
    for(i = 0; environ[i] != NULL; i++)
        puts(environ[i]);
    printf("\n");
    
    return 0;
}