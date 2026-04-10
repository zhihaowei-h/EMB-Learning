#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    //puts(getenv("TEST"));如果环境表中没有TEST环境变量,会出现段错误

    setenv("TEST", "ARE YOU OK?", 1);//添加一个环境变量
    
    puts(getenv("TEST"));
    
    return 0;
}