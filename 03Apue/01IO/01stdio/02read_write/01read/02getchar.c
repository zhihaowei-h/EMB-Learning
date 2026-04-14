#include <stdio.h>

int main(void) {
    int c; // 必须使用 int，而不是 char，为了容纳 EOF (-1)

    printf("请输入字符（按下 Ctrl+D 表示结束输入）:\n");

    /* * getchar() 从 stdin 读取一个字符
     * 注意：stdin 是行缓冲的，所以你输入一串字符后必须按回车，
     * 下面的循环才会开始一个一个处理缓冲区里的字符。
     */
    
    while ((c = getchar()) != EOF) {
        printf("读取到字符: '%c'，其 ASCII 码为: %d\n", c, c);
        // 如果遇到换行符，额外提醒一下
        if (c == '\n') {
            printf("--- 检测到行缓冲区刷新 ---\n");
        }
    }
    // 当检测到EOF时，getchar()返回EOF（通常是-1），循环结束
    printf("\n检测到 EOF，输入结束。\n");

    return 0;
}