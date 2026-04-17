/*===============================================
 *   文件名称：password_auth.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：密码认证模块
 *            基于shadow密码文件实现用户认证
 ================================================*/

#include "../inc/iot_gateway.h"

/*
 * 功能：验证用户密码（使用系统shadow文件）
 * 参数：username - 用户名
 *       password - 密码
 * 返回：成功返回0，失败返回-1
 */
int password_verify(const char *username, const char *password)
{
    struct spwd *sp;
    char *encrypted;
    
    if (username == NULL || password == NULL) {
        return -1;
    }
    
    // 获取shadow密码条目（需要root权限）
    sp = getspnam(username);
    if (sp == NULL) {
        log_write(LOG_WARN, "User not found: %s", username);
        return -1;
    }
    
    // 使用crypt函数加密输入的密码
    encrypted = crypt(password, sp->sp_pwdp);
    if (encrypted == NULL) {
        log_write(LOG_ERROR, "Password encryption failed");
        return -1;
    }
    
    // 比较加密后的密码
    if (strcmp(encrypted, sp->sp_pwdp) == 0) {
        log_write(LOG_INFO, "User authenticated: %s", username);
        return 0;
    } else {
        log_write(LOG_WARN, "Authentication failed for user: %s", username);
        return -1;
    }
}

/*
 * 功能：验证访问令牌
 * 参数：token - 令牌字符串
 * 返回：成功返回0，失败返回-1
 */
int token_verify(const char *token)
{
    if (token == NULL) {
        return -1;
    }
    
    // 去除可能的空白字符
    while (*token == ' ' || *token == '\t' || *token == '\n' || *token == '\r') {
        token++;
    }
    
    // 比较令牌
    if (strcmp(token, g_config.auth_token) == 0) {
        log_write(LOG_INFO, "Token authentication succeeded");
        return 0;
    } else {
        log_write(LOG_WARN, "Invalid token: %s", token);
        return -1;
    }
}
