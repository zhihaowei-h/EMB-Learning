#include <stdio.h>
#include <unistd.h>

#define MIN 100
#define MAX 300

// 判断一个数是否为质数的函数
static int is_prime(int num){
    int i = 0; // 循环变量

    sleep(1);

    // 如果num小于等于1,则不是质数
    if(num <= 1) return 0;
    // 如果num是2或3,则是质数
    if(num == 2 || num == 3) return 1;

    // 从2开始,一直循环到num的平方根,如果num能被i整除,则num不是质数
    for(i = 2; i <= num / i; i++){
        if(num % i == 0) return 0;
    }
    return 1;
}

int main(void){
    int i = 0; // 循环变量

    for(i = MIN; i <= MAX; i++){
        if(is_prime(i)) printf("%d Is A Prime Number\n", i);
    }

    return 0;
}