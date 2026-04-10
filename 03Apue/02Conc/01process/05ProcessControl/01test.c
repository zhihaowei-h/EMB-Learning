#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    int i = 0;//循环变量

    //srand(getpid());
    srand(getppid());

    for(i = 0; i < 5; i++)
        printf("%d\n", rand());

    return 0;
}