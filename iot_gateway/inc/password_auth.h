#ifndef PASSWORD_AUTH_H
#define PASSWORD_AUTH_H

#include "iot_gateway.h"

// 校验给定的用户名和明文密码是否匹配系统 /etc/shadow
int authenticate_user(const char *username, const char *password);

#endif // PASSWORD_AUTH_H