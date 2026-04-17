/*===============================================
 *   文件名称：base64.h
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月16日
 *   描    述：Base64编码解码
 *            用于API密钥编码
 ================================================*/

#ifndef __BASE64_H
#define __BASE64_H

#include <stddef.h>

/*
 * 功能：Base64编码
 * 参数：out - 输出缓冲区（需要足够大）
 *       in - 输入数据
 *       len - 输入数据长度
 * 返回：编码后的字符串长度
 */
int base64_encode(unsigned char *out, const unsigned char *in, size_t len);

/*
 * 功能：Base64编码（字符串版本）
 * 参数：out - 输出缓冲区
 *       in - 输入字符串
 * 返回：编码后的字符串长度
 */
int base64_encode_string(unsigned char *out, const unsigned char *in);

#endif /* __BASE64_H */
