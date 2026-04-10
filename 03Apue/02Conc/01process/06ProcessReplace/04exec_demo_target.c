#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("--- 进入 target 程序 ---\n");
    printf("我看到的我的名字 (argv[0]) 是: [%s]\n", argv[0]);

    // 根据不同的名字，表现出不同的“性格”
    if (strcmp(argv[0], "hello") == 0) {
        printf("=> 既然你叫我 hello，那我就执行【打招呼】功能：Hello, World!\n");
    } 
    else if (strcmp(argv[0], "bye") == 0) {
        printf("=> 既然你叫我 bye，那我就执行【告别】功能：Goodbye!\n");
    } 
    else {
        printf("=> 名字不认识，我什么都不做，直接退出。\n");
    }

    return 0;
}