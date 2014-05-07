/**
 * @file util.c
 * @brief 一些功能性函数的集合
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-07
 */

#include <stdio.h>
#include <assert.h>

#include "util.h"

void print_hex(const unsigned char *string, int length, FILE *file)
{
    assert(string != NULL);
    assert(length >= 0);

    int i;
    const unsigned char *base = string; 
    for (i = 0; i < length; i++)
    {
        fprintf(file, "%02X ", *(base + i));
    }

    fprintf(file, "\n");
}
