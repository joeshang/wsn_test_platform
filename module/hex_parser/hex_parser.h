/**
 * @file hex_parser.h
 * @brief 解析HEX格式文件
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-14
 */

#ifndef __HEX_PARSER_H__
#define __HEX_PARSER_H__

#include "../../common/common.h"

DECLS_BEGIN

typedef Ret (*HexLineDataCallBack)(int addr, const unsigned char *buf, int length, void *ctx);

struct _HexParser;
typedef struct _HexParser HexParser;

HexParser *hex_parser_create(HexLineDataCallBack lien_data_cb, void *cb_ctx);
void hex_parser_destroy(HexParser *thiz);

Ret hex_parser_process_file(HexParser *thiz, FILE *fd);

DECLS_END

#endif
