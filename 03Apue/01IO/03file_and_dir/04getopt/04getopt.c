#include <stdio.h>
#include <unistd.h>
  
int main(int argc, char *argv[]) {
    int opt;
      
    // optstring以'-'开头
    while ((opt = getopt(argc, argv, "-a:b")) != -1) {
        switch (opt) {
            case 'a':
                printf("选项 a, 参数: %s\n", optarg);
                break;
            case 'b':
                printf("选项 b\n");
                break;
            case 1:  // 非选项参数返回1
                printf("非选项参数: %s\n", optarg);
                break;
            case '?':
                printf("未知选项: %c\n", optopt);
                break;
        }
    }
      
    return 0;
}