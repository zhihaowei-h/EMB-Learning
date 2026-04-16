#include <stdio.h>
#include <unistd.h>
  
int main(int argc, char *argv[]) {
    int opt;
      
    while ((opt = getopt(argc, argv, "d::")) != -1) {
        switch (opt) {
            case 'd':
                if (optarg) printf("调试级别: %s\n", optarg);
                else printf("调试模式开启（默认级别）\n");
                break;
            case '?':
                printf("未知选项: %c\n", optopt);
                break;
        }
    }
      
    return 0;
}