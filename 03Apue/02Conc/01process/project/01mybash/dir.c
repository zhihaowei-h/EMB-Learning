#include "loghawk.h"

int dir_copy_recursive(const char *src_dir, const char *dest_dir) {
    struct stat st;
    if (stat(src_dir, &st) != 0) return -1;

    if (S_ISDIR(st.st_mode)) {
        mkdir(dest_dir, st.st_mode & 0777);
        DIR *dir = opendir(src_dir);
        if (!dir) return -1;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char src_path[1024], dst_path[1024];
            snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", dest_dir, entry->d_name);
            dir_copy_recursive(src_path, dst_path);
        }
        closedir(dir);
    } else {
        file_cp(src_dir, dest_dir);
    }
    return 0;
}

int dir_scan(const char *dirpath, int level) {
    DIR *dir = opendir(dirpath);
    if (!dir) { perror("无法打开目录"); return -1; }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        for (int i = 0; i < level; i++) printf("  |--");
        printf("%s\n", entry->d_name);

        if (entry->d_type == DT_DIR) {
            char sub[1024];
            snprintf(sub, sizeof(sub), "%s/%s", dirpath, entry->d_name);
            dir_scan(sub, level + 1);
        }
    }
    closedir(dir);
    return 0;
}

int file_search(const char *pattern) {
    glob_t gl;
    if (glob(pattern, GLOB_ERR, NULL, &gl) == 0) {
        for (size_t i = 0; i < gl.gl_pathc; i++) {
            printf("匹配到: %s\n", gl.gl_pathv[i]);
        }
        globfree(&gl);
        return 0;
    }
    printf("未找到匹配文件。\n");
    return -1;
}

int batch_operate(const char *pattern, char opt) {
    glob_t gl;
    int ret = glob(pattern, GLOB_ERR, NULL, &gl);
    
    if (ret == 0) {
        for (size_t i = 0; i < gl.gl_pathc; i++) {
            printf("\n--> 正在处理: %s\n", gl.gl_pathv[i]);
            switch(opt) {
                case 'l': file_stat_l(gl.gl_pathv[i]); break;
                case 'v': file_view(gl.gl_pathv[i]); break;
                case 's': file_du(gl.gl_pathv[i]); break;
                default: printf("不支持的批量操作符: %c\n", opt); break;
            }
        }
        globfree(&gl);
        return 0;
    } else if (ret == GLOB_NOMATCH) {
        // 加上这句，以后匹配不到文件就不会默默退出了！
        printf("未找到匹配模式 '%s' 的文件。\n", pattern);
    } else {
        perror("glob 批量匹配出错");
    }
    
    return -1;
}