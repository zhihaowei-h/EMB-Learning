#include "loghawk.h"

char get_file_type(mode_t st_mode) {
    if (S_ISREG(st_mode))  return '-';
    if (S_ISDIR(st_mode))  return 'd';
    if (S_ISCHR(st_mode))  return 'c';
    if (S_ISBLK(st_mode))  return 'b';
    if (S_ISFIFO(st_mode)) return 'p';
    if (S_ISLNK(st_mode))  return 'l';
    if (S_ISSOCK(st_mode)) return 's';
    return '?';
}

char *get_file_permission(mode_t st_mode, char *perm) {
    strcpy(perm, "---------");
    if (st_mode & S_IRUSR) perm[0] = 'r';
    if (st_mode & S_IWUSR) perm[1] = 'w';
    if (st_mode & S_IXUSR) perm[2] = 'x';
    if (st_mode & S_ISUID) perm[2] = (st_mode & S_IXUSR) ? 's' : 'S'; // 特殊权限 SUID

    if (st_mode & S_IRGRP) perm[3] = 'r';
    if (st_mode & S_IWGRP) perm[4] = 'w';
    if (st_mode & S_IXGRP) perm[5] = 'x';
    if (st_mode & S_ISGID) perm[5] = (st_mode & S_IXGRP) ? 's' : 'S'; // 特殊权限 SGID

    if (st_mode & S_IROTH) perm[6] = 'r';
    if (st_mode & S_IWOTH) perm[7] = 'w';
    if (st_mode & S_IXOTH) perm[8] = 'x';
    if (st_mode & S_ISVTX) perm[8] = (st_mode & S_IXOTH) ? 't' : 'T'; // 特殊权限 Sticky
    
    return perm;
}

int file_stat_l(const char *filename) {
    struct stat st;
    if (lstat(filename, &st) == -1) { perror("获取属性失败"); return -1; }

    char type = get_file_type(st.st_mode);
    char perm[11];
    get_file_permission(st.st_mode, perm);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    
    char time_str[64];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);

    printf("%c%s %ld %s %s %8ld %s %s\n", type, perm, st.st_nlink, 
           pw ? pw->pw_name : "UID", gr ? gr->gr_name : "GID",
           st.st_size, time_str, filename);
    return 0;
}

long du_calc(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) return 0;

    // st_blocks 的单位默认是 512 Bytes
    long total_blocks = st.st_blocks;

    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                char sub[1024];
                snprintf(sub, sizeof(sub), "%s/%s", path, entry->d_name);
                total_blocks += du_calc(sub);
            }
            closedir(dir);
        }
    }
    return total_blocks;
}

int file_du(const char *path) {
    long blocks = du_calc(path);
    double size_kb = (blocks * 512.0) / 1024.0;
    
    if (size_kb < 1024) {
        printf("%.2f KB\t%s\n", size_kb, path);
    } else if (size_kb < 1024 * 1024) {
        printf("%.2f MB\t%s\n", size_kb / 1024.0, path);
    } else {
        printf("%.2f GB\t%s\n", size_kb / (1024.0 * 1024.0), path);
    }
    return 0;
}