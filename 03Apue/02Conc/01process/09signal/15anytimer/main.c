#include <stdio.h>
#include <unistd.h>
#include "anytimer.h"

void sig_handler(void *arg){
    write(1, arg, 1); // 增加 (char *) 强转
}

int main(void){
    anytimer_init(10, sig_handler, (void *)"!");
    anytimer_init(1, sig_handler, (void *)"!");
    anytimer_init(15, sig_handler, (void *)"!");

    while(1){
        write(1, "*", 1);
        sleep(1);
    }

    return 0;
}