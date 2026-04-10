#include "loghawk.h"

static struct termios old_term;

void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

void login_timeout_handler(int none) {
    (void)none; // 显式忽略该参数，消除编译警告
    printf("\n[系统提示] 登录超时(%d秒)！\n", LOGIN_TIMEOUT);
    reset_terminal();
    exit(EXIT_FAILURE);
}

char *get_passwd_input(const char *prompt) {
    static char passwd[128];
    printf("%s", prompt);
    fflush(stdout);

    struct termios new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~ECHO; // 关闭回显
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    if (fgets(passwd, sizeof(passwd), stdin) != NULL) {
        passwd[strcspn(passwd, "\n")] = 0; // 去除换行符
    }
    
    reset_terminal();
    printf("\n");
    return passwd;
}

int verify_user_passwd(const char *user, const char *passwd) {
    // 模拟写死的校验，实际生产可读取 /etc/shadow
    if (strcmp(user, "admin") == 0 && strcmp(passwd, "123456") == 0) {
        return 1;
    }
    return 0;
}

void show_login_prompt() {
    printf("=== 欢迎使用 Embedded File Factory ===\n");
    printf("请在 %d 秒内完成身份认证\n", LOGIN_TIMEOUT);
}

int login_authenticate() {
    show_login_prompt();
    
    signal(SIGALRM, login_timeout_handler);
    alarm(LOGIN_TIMEOUT); // 启动超时定时器

    char user[64];
    printf("用户名: ");
    fflush(stdout);
    if (fgets(user, sizeof(user), stdin) == NULL) return -1;
    user[strcspn(user, "\n")] = 0;

    char *passwd = get_passwd_input("密码: ");
    
    alarm(0); // 认证结束，取消定时器

    if (verify_user_passwd(user, passwd)) {
        printf("认证成功！进入系统...\n");
        return 0;
    } else {
        printf("用户名或密码错误！\n");
        return -1;
    }
}