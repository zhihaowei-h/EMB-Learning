#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    puts(getenv(argv[1])); // 通过getenv函数获取环境变量值，参数为环境变量名
    return 0;
}