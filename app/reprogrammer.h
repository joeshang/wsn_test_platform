/**
 * @file reprogramer.h
 * @brief 负责处理重编程流程
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-14
 */

#ifndef __REPROGRAMER_H__
#define __REPROGRAMER_H__

#include "../common/common.h"
#include "gather_board.h"

DECLS_BEGIN

struct _Reprogrammer;
typedef struct _Reprogrammer Reprogrammer;

Reprogrammer *reprogrammer_create(GatherBoard *gather_board);
void reprogrammer_destroy(Reprogrammer *thiz);
Ret  reprogrammer_process(Reprogrammer *thiz, int socket);

DECLS_END

#endif
