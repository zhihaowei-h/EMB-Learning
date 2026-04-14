#include <stdio.h>
#include <unistd.h>

int main(void){
    int i = 0;
    int count = 0; // 循环的次数

    while(1){
        scanf("%x", &i);  // stdin缓冲区1 \n
         if(i == 0) break;
        printf("i = %x\n", i);
        count++;
    }
    
    printf("循环的次数：%d\n", count);
    return 0;
}