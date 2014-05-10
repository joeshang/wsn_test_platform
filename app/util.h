/**
 * @file util.h
 * @brief 一些功能性函数的集合
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-07
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "../common/common.h"

DECLS_BEGIN

/**
 * @brief 打印字符串的16进制格式，打印完规定长度后输出回车
 *
 * @param string 需要打印的字符串
 * @param length 字符串长度
 * @param output_file 打印输出的文件，可以是标准输出
 */
void print_hex(const unsigned char *string, int length, FILE *output_file);

/**
 * @brief 在拷贝时处理delimiter字符
 *
 * @param dest 目标缓冲区
 * @param src 源缓冲区
 * @param copy_len 源缓冲区要拷贝的长度
 *
 * @return 返回最终拷贝个数
 */
uint32_t copy_with_process_delimiter(uint8_t *dest, const uint8_t *src, uint32_t copy_len);

/**
 * @brief 计算16位宽的CRC
 *
 * @param buf 要计算的缓冲区
 * @param buf_size 缓冲区长度
 *
 * @return 返回16位宽的CRC
 */
uint16_t crc_16_width(uint8_t *buf, uint32_t buf_size);

/**
 * @brief 偶校验
 *
 * @param buf 要校验的缓冲区
 * @param buf_size 缓冲区长度
 *
 * @return 返回YES代表校验成功，返回FALSE代表失败
 */
uint8_t even_check(uint8_t *buf, uint32_t buf_size);

DECLS_END

#endif
