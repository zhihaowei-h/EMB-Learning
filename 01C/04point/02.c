// 正确示例（因为我们想要修改的是一级指针的值，所以应该传二级指针）
#include <stdio.h>
#include <stdlib.h>

// 正确示范：传入二级指针（即指针的地址）
void correct_allocate(int **p) {
    // *p 就代表了 main 函数里的那个 my_p 变量本身
    // 给 *p 赋值，就是直接把新地址写进了 main 函数的 my_p 中
    *p = (int *)malloc(sizeof(int)); 
    
    // **p 就是给新分配的这块内存里面塞数据
    **p = 100;
}

int main() {
    int *my_p = NULL;
    
    // 关键点 1：一定要传 my_p 的“地址”（&my_p）进去
    correct_allocate(&my_p); 
    
    // 此时 my_p 已经拿到了真实的堆内存地址！
    if (my_p != NULL) {
        printf("成功分配！my_p 指向的值是: %d\n", *my_p);
    }
    
    // 用完记得释放
    free(my_p);
    
    return 0;
}