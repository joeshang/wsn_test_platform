/**
 * @file hex_parser.c
 * @brief 解析HEX格式文件
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-14
 */

#include <stdio.h>
#include <stdlib.h>

#include "hex_parser.h"

#define LINE_BUF_SIZE                   256

#define HEX_DELIMITER_WIDTH             1
#define HEX_HEADER_WIDTH                8
/* HEX为16进制的ascii表示，因此一个字节数据在HEX长度为2 */
#define HEX_DATA_UNIT_WIDTH             2   

/* HEX详细格式见：en.wikipedia.org/wiki/Intel_HEX */
#define HEX_TYPE_DATA_RECORD            0
#define HEX_TYPE_EOF_RECORD             1
#define HEX_TYPE_EXT_SEG_ADDR_RECORD    2
#define HEX_TYPE_START_SEG_ADDR_RECORD  3
#define HEX_TYPE_EXT_LINE_ADDR_RECORD   4
#define HEX_TYPE_START_LINE_ADDR_RECORD 5


struct _HexParser
{
    HexLineDataCallBack line_data_cb;
    void *cb_ctx;
};

static Ret hex_parser_process_per_line(HexParser *thiz, const char *line_buf, int line)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(line_buf != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    const char *s = line_buf;
    unsigned int type;
    unsigned int addr;
    unsigned int data_length;
    unsigned char data_buf[LINE_BUF_SIZE];

    /* 检查第一个字符是否为:（HEX格式每一行的行首标识符 */
    if (*s != ':')
    {
        fprintf(stderr, "invalid delimiter in line %d -> %s\n", line, line_buf);
        return RET_FAIL;
    }
    s += HEX_DELIMITER_WIDTH;

    /* 解析头部，格式为：< DataLength(2B) | Address(4B) | Type(2B) > */
    if (sscanf(s, "%02x%04x%02x", &data_length, &addr, &type) != 3)
    {
        fprintf(stderr, "invalid header in line %d -> %s\n", line, line_buf);
        return RET_FAIL;
    }
    s += HEX_HEADER_WIDTH;

    switch (type)
    {
        case HEX_TYPE_DATA_RECORD:
        {
            unsigned char calc_sum;
            unsigned int  check_sum;

            /* 检测数据长度有效性 */
            if (data_length >= LINE_BUF_SIZE)
            {
                fprintf(stderr, "invalid data length in line %d -> %s\n", line, line_buf);
                return RET_FAIL;
            }

            calc_sum = data_length + addr + (addr >> 8) + type;

            unsigned int i;
            unsigned int data;
            for (i = 0; i < data_length; i++)
            {
                if (sscanf(s, "%02x", &data) != 1)
                {
                    fprintf(stderr, "invalid data in line %d -> %s\n", line, line_buf);
                    return RET_FAIL;
                }

                s += HEX_DATA_UNIT_WIDTH;
                data_buf[i] = data;
                calc_sum += data;
            }

            if (sscanf(s, "%02x", &check_sum) != 1)
            {
                fprintf(stderr, "invalid checksum in line %d -> %s\n", line, line_buf);
                return RET_FAIL;
            }

            if ((calc_sum + check_sum) & 0xFF)
            {
                fprintf(stderr, "dismatch checksum in line %d -> %s\n", line, line_buf);
                return RET_FAIL;
            }

            ret = thiz->line_data_cb(addr, data_buf, data_length, thiz->cb_ctx);
            break;
        }
        case HEX_TYPE_EOF_RECORD:
            ret = RET_STOP;
            break;
        case HEX_TYPE_EXT_SEG_ADDR_RECORD:
        case HEX_TYPE_START_SEG_ADDR_RECORD:
        case HEX_TYPE_EXT_LINE_ADDR_RECORD:
        case HEX_TYPE_START_LINE_ADDR_RECORD:
            ret = RET_OK;
            break;
        default:
            fprintf(stderr, "format violation(invalid type) in line %d -> %s\n", line, line_buf);
            ret = RET_FAIL;
            break;
    }

    return ret;
}

HexParser *hex_parser_create(HexLineDataCallBack line_data_cb, void *cb_ctx)
{
    HexParser *thiz = (HexParser *)malloc(sizeof(HexParser));

    if (thiz != NULL)
    {
        thiz->line_data_cb = line_data_cb;
        thiz->cb_ctx = cb_ctx;
    }

    return thiz;
}

void hex_parser_destroy(HexParser *thiz)
{
    if (thiz != NULL)
    {
        thiz->line_data_cb = NULL;
        thiz->cb_ctx = NULL;

        free(thiz);
    }
}

Ret hex_parser_process_file(HexParser *thiz, FILE *fd)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(fd != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    int line = 0;
    char line_buf[LINE_BUF_SIZE];

    rewind(fd);

    while (fgets(line_buf, LINE_BUF_SIZE, fd) != NULL)
    {
        line++;
        ret = hex_parser_process_per_line(thiz, line_buf, line);
        if (ret != RET_OK)
        {
            break;
        }
    }

    return ret;
}
