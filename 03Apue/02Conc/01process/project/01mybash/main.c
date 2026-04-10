#include "loghawk.h"

void show_help(const char *prog_name) {
    printf("=================Embedded File Factory(嵌入式文件工厂)=================\n");
    printf("使用方法: %s [选项] 目标文件/目录\n", prog_name);
    printf("  -l <路径>            查看文件详细属性\n");
    printf("  -c <源> <目标>       复制普通文件\n");
    printf("  -v <路径>            查看文件内容\n");
    printf("  -d <目录>            查看目录树\n");
    printf("  -s <路径>            统计磁盘占用\n");
    printf("  -R <源> <目标>       递归复制目录\n");
    printf("  -r <路径>            递归删除文件/目录\n");
    printf("  -n <旧> <新>         重命名\n");
    printf("  -f <模式>            文件搜索(如 *.c)\n");
    printf("  -m <权限> <路径>     修改权限(如 0755)\n");
    printf("  -b <模式> <操作>     批量操作(操作: l, v, s)\n");
    printf("  -o <日志>            输出重定向\n");
    printf("  -h                   查看帮助\n");
    printf("=======================================================================\n");
}

int main(int argc, char *argv[]) {
    // 【已移除登录认证模块】

    if (argc < 2) {
        show_help(argv[0]);
        return EXIT_SUCCESS;
    }

    int opt;
    // 解析带有重定向的提前处理
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_redirect(argv[i+1]);
            break;
        }
    }

    // 重置 getopt
    optind = 1; 
    while ((opt = getopt(argc, argv, "hl:c:v:d:s:R:r:n:f:m:b:o:")) != -1) {
        switch (opt) {
            case 'h': show_help(argv[0]); break;
            case 'l': file_stat_l(optarg); break;
            case 'v': file_view(optarg); break;
            case 'd': dir_scan(optarg, 0); break;
            case 's': file_du(optarg); break;
            case 'r': file_rm(optarg); break;
            case 'f': file_search(optarg); break;
            case 'o': /* 已提前处理 */ break;
            case 'c':
                if (optind < argc) { file_cp(optarg, argv[optind++]); }
                else { fprintf(stderr, "-c 需要两个参数\n"); }
                break;
            case 'R':
                if (optind < argc) { dir_copy_recursive(optarg, argv[optind++]); }
                else { fprintf(stderr, "-R 需要两个参数\n"); }
                break;
            case 'n':
                if (optind < argc) { file_rename(optarg, argv[optind++]); }
                else { fprintf(stderr, "-n 需要两个参数\n"); }
                break;
            case 'm':
                if (optind < argc) {
                    mode_t mode = strtol(optarg, NULL, 8);
                    file_chmod(argv[optind++], mode);
                } else { fprintf(stderr, "-m 需要权限和路径\n"); }
                break;
            case 'b':
                if (optind < argc) { batch_operate(optarg, argv[optind++][0]); }
                else { fprintf(stderr, "-b 需要模式和操作符\n"); }
                break;
            default: show_help(argv[0]); break;
        }
    }
    return 0;
}