#include <stdio.h>

int main(void){
    char buf[1024];
    int count = 0; // 循环次数
    while (fgets(buf, 3, stdin) != NULL) {
        fputs(buf, stdout);
        count++;
    }
    printf("Number of lines read: %d\n", count);

    return 0;
}