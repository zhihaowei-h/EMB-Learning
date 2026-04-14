// 1.3.7 密码校验与安全需求
#include "password_auth.h"
#include <shadow.h>
#include <crypt.h>
#include <string.h>
#include <errno.h>

int authenticate_user(const char *username, const char *password) {
    struct spwd spbuf;
    struct spwd *sp;
    char buffer[1024]; // 用于给 getspnam_r 存放字符串的临时空间

    // 1. 线程安全地获取 shadow 记录
    int ret = getspnam_r(username, &spbuf, buffer, sizeof(buffer), &sp);
    if (ret != 0 || sp == NULL) {
        LOG_WARN("Auth failed: User '%s' not found or read /etc/shadow permission denied", username);
        return 0; // 失败
    }

    // 2. 线程安全地进行密码哈希计算
    struct crypt_data cdata;
    cdata.initialized = 0; // 必须置 0，告诉 crypt_r 进行初始化
    
    // sp->sp_pwdp 包含了 "$id$salt$encrypted" 格式的哈希字符串
    // crypt_r 会自动提取其中的 salt 并对明文 password 进行单向哈希
    char *encrypted = crypt_r(password, sp->sp_pwdp, &cdata);
    if (encrypted == NULL) {
        LOG_ERROR("crypt_r function failed");
        return 0;
    }

    // 3. 比较哈希结果
    if (strcmp(encrypted, sp->sp_pwdp) == 0) {
        LOG_INFO("Client authenticated successfully as user: '%s'", username);
        return 1; // 认证通过
    } else {
        LOG_WARN("Auth failed: Incorrect password for user '%s'", username);
        return 0; // 密码错误
    }
}