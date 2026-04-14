#include <stdio.h>
#include <unistd.h>
  
int main(int argc, char *argv[]) {
    int opt;
      
    while ((opt = getopt(argc, argv, "f:o:")) != -1) {
        switch (opt) {
            case 'f':
                printf("输入文件: %s\n", optarg);
                break;
            case 'o':
                printf("输出文件: %s\n", optarg);
                break;
            case '?':
                if (optopt == 'f' || optopt == 'o')
                    printf("选项 -%c 需要参数\n", optopt);
                else
                    printf("未知选项: %c\n", optopt);
                break;
        }
    }
      
    return 0;
}