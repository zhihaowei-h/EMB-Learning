#include <stdio.h>
#include <setjmp.h>

jmp_buf env;//存储有关调用环境的各种信息

int div(int num1, int num2)
{
    if(num2 == 0)//判断除数是否为0
        longjmp(env, 1);//跳转
    return num1 / num2;
}

int main(void)
{
    int num1 = 0, num2 = 0;//存储输入的整数
    int sum = 0;//存储计算的结果

    if(setjmp(env) == 0)//设置跳转的位置(判断是否是手动调用)
        printf("请输入两个整型数 : ");
    else//如果是跳转过来的
        printf("请重新输入两个整型数(注意:除数不能为0) : ");

    scanf("%d-%d", &num1, &num2);//录入两个整型数

    sum = div(num1, num2);//调用自己实现的函数进行除法计算

    printf("%d / %d = %d\n", num1, num2, sum);

    return 0;
}
