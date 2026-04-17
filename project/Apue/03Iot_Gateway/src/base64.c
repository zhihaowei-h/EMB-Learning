/*===============================================
 *   文件名称：base64.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月16日
 *   描    述：Base64编码实现
 ================================================*/

#include "base64.h"
#include <string.h>

// Base64编码表
static const char base64_table[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
 * 功能：Base64编码
 * 参数：out - 输出缓冲区
 *       in - 输入数据
 *       len - 输入数据长度
 * 返回：编码后的长度
 */
int base64_encode(unsigned char *out, const unsigned char *in, size_t len)
{
    unsigned char *pos;
    const unsigned char *end, *in_pos;
    int line_len = 0;
    
    if (out == NULL || in == NULL) {
        return -1;
    }
    
    pos = out;
    end = in + len;
    in_pos = in;
    
    while (end - in_pos >= 3) {
        *pos++ = base64_table[in_pos[0] >> 2];
        *pos++ = base64_table[((in_pos[0] & 0x03) << 4) | (in_pos[1] >> 4)];
        *pos++ = base64_table[((in_pos[1] & 0x0f) << 2) | (in_pos[2] >> 6)];
        *pos++ = base64_table[in_pos[2] & 0x3f];
        in_pos += 3;
        line_len += 4;
    }
    
    if (end - in_pos) {
        *pos++ = base64_table[in_pos[0] >> 2];
        if (end - in_pos == 1) {
            *pos++ = base64_table[(in_pos[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = base64_table[((in_pos[0] & 0x03) << 4) | (in_pos[1] >> 4)];
            *pos++ = base64_table[(in_pos[1] & 0x0f) << 2];
        }
        *pos++ = '=';
        line_len += 4;
    }
    
    *pos = '\0';
    
    return pos - out;
}

/*
 * 功能：Base64编码（字符串版本）
 * 参数：out - 输出缓冲区
 *       in - 输入字符串
 * 返回：编码后的长度
 */
int base64_encode_string(unsigned char *out, const unsigned char *in)
{
    return base64_encode(out, in, strlen((const char *)in));
}
