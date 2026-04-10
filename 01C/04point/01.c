// 我们要做的事情很简单：在 main 函数里有一个空指针 my_p，我们想通过调用一个函数，给 指针my_kaap 分配一块内存，并存入一个数字 100。
// 错误示例
#include <stdio.h>
#include <stdlib.h>

// 错误示范：试图用一级指针分配内存
void wrong_allocate(int *p) {
    // 这里的 p 只是 main 函数中 p 的一个“副本”
    p = (int *)malloc(sizeof(int)); 
    *p = 100;
    // 函数结束后，这个副本 p 被销毁，新分配的内存地址丢失（内存泄漏）
}

int main() {
    int *my_p = NULL; // my_p 是一个一级指针，指向一个整数，但目前它是 NULL
    
    // 传值调用，把 NULL 传进去了
    wrong_allocate(my_p); 
    
    // 此时 my_p 依然是 NULL！下面这行代码会导致程序崩溃（段错误）
    // printf("my_p 的值是: %d\n", *my_p); 
    
    return 0;
}