/**
 * @file packet_transfer.h
 * @brief 负责接收并解析上位机发送命令，发送给FPGA板，并从FPGA板读取数据，发回上位机
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#ifndef __PACKET_TRANSFER_H__
#define __PACKET_TRANSFER_H__

#include "../common/common.h"
#include "gather_board.h"

DECLS_BEGIN

struct _PacketTransfer;
typedef struct _PacketTransfer PacketTransfer;

PacketTransfer *packet_transfer_create(GatherBoard *gather_board);
void packet_transfer_destroy(PacketTransfer *thiz);
Ret  packet_transfer_process(PacketTransfer *thiz, int socket);
Ret  packet_transfer_reset(PacketTransfer *thiz);

DECLS_END

#endif
