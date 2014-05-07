/**
 * @file gather_board.h
 * @brief 封装与FPGA板的通信（包括GPIO跟SPI）
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#ifndef __GATHER_BOARD_H__
#define __GATHER_BOARD_H__

#include "common.h"
#include "oint_gpio.h"

DECLS_BEGIN

struct _GatherBoard;
typedef struct _GatherBoard GatherBoard;

GatherBoard *gather_board_create();
void gather_board_destroy(GatherBoard *thiz);

Ret  gather_board_led_red(GatherBoard *thiz, int led_index, int value);
Ret  gather_board_led_green(GatherBoard *thiz, int led_index, int value);
Ret  gather_board_current_switch(GatherBoard *thiz, int value);
Ret  gather_board_reset_fpga(GatherBoard *thiz);

DECLS_END

#endif
