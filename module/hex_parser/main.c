/**
 * @file main.c
 * @brief HexParser测试主函数
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-14
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "hex_parser.h"

#define HEX_FILE_PATH           "sample/program.ihex"

static Ret hex_parser_cb(int addr, const unsigned char *buf, int length, void *ctx)
{
    printf("addr: %X, data:", addr);
    int i;
    for (i = 0; i < length; i++)
    {
        printf(" %02X", buf[i]);
    }
    printf("\n");

    return RET_OK;
}

int main(int argc, char *argv[])
{
    FILE *hex_fd;

    if ((hex_fd = fopen(HEX_FILE_PATH, "r")) == NULL)
    {
        fprintf(stderr, "open file %s failed: %s\n", HEX_FILE_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }

    HexParser *parser = hex_parser_create(hex_parser_cb, NULL);

    hex_parser_process_file(parser, hex_fd);

    hex_parser_destroy(parser);

    return 0;
}

