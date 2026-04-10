#ifndef EFF_H
#define EFF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <glob.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

#define BUFFER_SIZE 4096 // 默认缓冲区大小
#define LOGIN_TIMEOUT 30 // 登录超时时间(秒)

// ================= 主模块 =================
void show_help(const char *prog_name);

// ================= 认证模块 (auth.c) =================
int login_authenticate();
char *get_passwd_input(const char *prompt);
int verify_user_passwd(const char *user, const char *passwd);
void login_timeout_handler(int sig);
void reset_terminal();
void show_login_prompt();

// ================= 文件模块 (file.c) =================
int file_cp(const char *src, const char *dest);
int file_view(const char *filename);
int file_rm(const char *path);
int file_rename(const char *old_path, const char *new_path);
int file_chmod(const char *path, mode_t mode);
int output_redirect(const char *log_path);

// ================= 目录模块 (dir.c) =================
int dir_copy_recursive(const char *src_dir, const char *dest_dir);
int dir_scan(const char *dirpath, int level);
int file_search(const char *pattern);
int batch_operate(const char *pattern, char opt);

// ================= 属性与统计模块 (stat.c) =================
char get_file_type(mode_t st_mode);
char *get_file_permission(mode_t st_mode, char *perm);
int file_stat_l(const char *filename);
long du_calc(const char *path);
int file_du(const char *path);

#endif // EFF_H