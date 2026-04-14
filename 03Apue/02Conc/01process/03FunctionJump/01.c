#include <stdio.h>
#include <setjmp.h>

jmp_buf env;//存储有关调用环境的各种信息

// 实现一个除法函数，如果除数为0则调用longjmp进行跳转
int div(int num1, int num2){
    // 如果除数为0
    if(num2 == 0) longjmp(env, 1); // 调用longjmp函数进行跳转，参数1表示跳转回来时setjmp的返回值
    return num1 / num2;
}

int main(void){
    int num1 = 0, num2 = 0; // 存储输入的整数
    int sum = 0; // 存储计算的结果

    // setjmp函数用于保存当前的调用环境，第一次调用时一定返回0，如果通过longjmp跳转回来则返回非0值
    if(setjmp(env) == 0)
        printf("请输入两个整型数 : ");
    else // 如果是跳转过来的
        printf("请重新输入两个整型数(注意:除数不能为0): ");

    scanf("%d-%d", &num1, &num2); // 录入两个整型数
    sum = div(num1, num2); // 调用自己实现的函数进行除法计算
    printf("%d / %d = %d\n", num1, num2, sum);

    return 0;
}