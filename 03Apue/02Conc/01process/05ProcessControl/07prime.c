#include <stdio.h>
#include <unistd.h>

#define MIN 100
#define MAX 300

static int is_prime(int num)
{
    int i = 0;//循环变量

    sleep(1);

    if(num <= 1)//判断num是否小于等于1(是否不是质数)
        return 0;
    if(num == 2 || num == 3)
        return 1;

    for(i = 2; i <= num / i; i++)
    {
        if(num % i == 0)
            return 0;
    }
    return 1;
}

int main(void)
{
    int i = 0;//循环变量

    for(i = MIN; i <= MAX; i++)
    {
        if(is_prime(i))
            printf("%d Is A Prime Number\n", i);
    }

    return 0;
}