#include "loghawk.h"

int file_cp(const char *src, const char *dest) {
    int fd_in = open(src, O_RDONLY);
    if (fd_in < 0) { perror("打开源文件失败"); return -1; }

    int fd_out = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) { perror("创建目标文件失败"); close(fd_in); return -1; }

    char buf[BUFFER_SIZE];
    ssize_t n;
    while ((n = read(fd_in, buf, sizeof(buf))) > 0) {
        if (write(fd_out, buf, n) != n) {
            perror("写入文件异常");
            break;
        }
    }
    close(fd_in);
    close(fd_out);
    return 0;
}

int file_view(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("无法打开文件"); return -1; }
    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        printf("%s", buf);
    }
    fclose(fp);
    return 0;
}

int file_rm(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) { perror("获取路径信息失败"); return -1; }

    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) return -1;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char sub[1024];
            snprintf(sub, sizeof(sub), "%s/%s", path, entry->d_name);
            file_rm(sub); // 递归删除
        }
        closedir(dir);
        return rmdir(path);
    } else {
        return unlink(path);
    }
}

int file_rename(const char *old_path, const char *new_path) {
    if (rename(old_path, new_path) != 0) {
        perror("重命名失败");
        return -1;
    }
    return 0;
}

int file_chmod(const char *path, mode_t mode) {
    if (chmod(path, mode) != 0) {
        perror("修改权限失败");
        return -1;
    }
    printf("已成功修改 %s 权限\n", path);
    return 0;
}

int output_redirect(const char *log_path) {
    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) { perror("打开日志文件失败"); return -1; }
    
    // dup2 替换标准输出流
    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("重定向失败");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}