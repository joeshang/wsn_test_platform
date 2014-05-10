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
#include "oint_protocol.h"

void print_hex(const uint8_t *string, int length, FILE *file)
{
    assert(string != NULL);
    assert(length >= 0);

    int i;
    const uint8_t *base = string; 
    for (i = 0; i < length; i++)
    {
        fprintf(file, "%02X ", *(base + i));
    }

    fprintf(file, "\n");
}

uint32_t copy_with_process_delimiter(uint8_t *dest, const uint8_t *src, uint32_t copy_len)
{
    assert(dest != NULL);
    assert(src != NULL);

    uint8_t i = 0;
    uint8_t value;
    uint32_t index = 0;

    for (i = 0; i < copy_len; i++)
    {
        value = src[i];
        if (value == NODE_DELIMITER || value == NODE_ESCAPE)
        {
            dest[index++] = NODE_ESCAPE;
            dest[index++] = NODE_DELIMITER_TRANSFER(value);
        }
        else
        {
            dest[index++] = value;
        }
    }

    return index;
}

uint16_t crc_16_width(uint8_t *buf, uint32_t buf_size)
{
    uint16_t crc = 0;
    
    // TODO

    return crc;
}

uint8_t even_check(uint8_t *buf, uint32_t buf_size)
{
    uint32_t i;
    uint8_t result = buf[0];

    for (i = 1; i < buf_size; i++)
    {
        result ^= buf[i];
    }

    if (result)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
