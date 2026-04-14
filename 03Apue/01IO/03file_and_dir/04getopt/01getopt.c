#include <stdio.h>
#include <unistd.h>
  
int main(int argc, char *argv[]) {
    int opt;
      
    while ((opt = getopt(argc, argv, "abcd")) != -1) {
        switch (opt) {
            case 'a':
                printf("选项 a\n");
                break;
            case 'b':
                printf("选项 b\n");
                break;
            case 'c':
                printf("选项 c\n");
                break;
            case 'd':
                printf("选项 d\n");
                break;
            case '?':
                printf("未知选项: %c\n", optopt);
                break;
        }
    }
      
    return 0;
}