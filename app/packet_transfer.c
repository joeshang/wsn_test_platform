/**
 * @file packet_transfer.c
 * @brief 负责接收并解析上位机发送命令，发送给FPGA板，并从FPGA板读取数据，发回上位机
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <memory.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "packet_transfer.h"
#include "oint_protocol.h"
#include "util.h"
#include "../common/dlist.h"

#define NODE_NUM        8

typedef struct _Command
{
    int port_id;
    int type;
    int board_packet_len;
    uint8_t board_packet[CMD_B_PACKET_MAX_SIZE];
}Command;

struct _PacketTransfer
{
    int uploader_socket;    // 上位机连接socket(TCP)
    /*int display_socket;     // 本机Qt显示socket(UDP)*/
    DList *command_set[NODE_NUM];
};

static void handle_command(PacketTransfer *thiz, Command *command);
static Ret  process_one_command(PacketTransfer *thiz);
static Ret  process_one_response(PacketTransfer *thiz);

static void handle_command(PacketTransfer *thiz, Command *command)
{
    uint8_t payload_data;

    if (command->type == PAYLOAD_TYPE_CNTL_POWER_SWITCH)
    {
        uint8_t to_board;
        payload_data = command->board_packet[CMD_B_TYPE_INDEX + 1];

        if (payload_data == 0)  /* 关闭节点 */
        {
            to_board = CMD_B_NODE_CLOSE_BASE + command->port_id - 1;
        }
        else /* 打开节点 */
        {
            to_board = CMD_B_NODE_OPEN_BASE + command->port_id - 1;
        }

        // TODO: gather_board_write(board, &to_board, 1);
    }
    else
    {
        // TODO: gather_board_write(board, &command->board_packet, command->board_packet_len);
        
        int wait_for_response = TRUE;
        switch (command->type)
        {
            case PAYLOAD_TYPE_QUERY_CURRENT:
            case PAYLOAD_TYPE_CNTL_POWER_SWITCH:
                wait_for_response = FALSE;
            default:
                wait_for_response = TRUE;
                break;
        }

        if (wait_for_response == TRUE)
        {
            dlist_append(thiz->command_set[command->port_id - 1], (void *)command);
        }
    }

    debug("[PacketTransfer]: send to node %d ok\n", command->port_id);
}

static Ret  process_one_command(PacketTransfer *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    int n;
    uint16_t command_size = 0;

    Command *command = (Command *)malloc(sizeof(Command));
    if (command == NULL)
    {
        fprintf(stderr, "malloc command failed\n");
        goto fail;
    }

    /* 1. 读取命令包的长度跟PortID */ 
    n = read(thiz->uploader_socket, &command_size, CMD_S_HEADER_LEN);
    if (n == -1)
    {
        fprintf(stderr, "data uploader read command content failed\n");
        goto fail;
    }
    else if (n == 0)
    {
        fprintf(stderr, "peer client close connection\n");
        goto fail;
    }
    command_size = ntohs(command_size);

    /* 2. 读取命令包中发送至FPGA板的命令 */
    n = read(thiz->uploader_socket, command->board_packet, command_size);
    if (n == -1)
    {
        fprintf(stderr, "data uploader read command content failed\n");
        goto fail;
    }
    else if (n == 0)
    {
        fprintf(stderr, "peer client close connection\n");
        goto fail;
    }

    /* 3. 根据从上位机接收的命令包初始化Command */
    command->board_packet_len = command_size;
    command->port_id = command->board_packet[CMD_B_PORT_ID_INDEX];
    command->type = command->board_packet[CMD_B_TYPE_INDEX];

    debug("[PacketTransfer]: receive command -> ");
#ifdef _DEBUG
    print_hex(command->board_packet, command->board_packet_len, stdout);
#endif

    handle_command(thiz, command);        

    return RET_OK;
fail:
    SAFE_FREE(command);
    return RET_FAIL;
}

static Ret  process_one_response(PacketTransfer *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    return RET_OK;
}

static void command_destroy_cb(void *ctx, void *data)
{
    if (data != NULL)
    {
        free(data);
    }
}

PacketTransfer *packet_transfer_create()
{
    PacketTransfer *thiz = (PacketTransfer *)malloc(sizeof(PacketTransfer));

    if (thiz != NULL)
    {
        thiz->uploader_socket = -1;

        int i;
        for (i = 0; i < NODE_NUM; i++)
        {
            thiz->command_set[i] = dlist_create(command_destroy_cb, NULL);
        }
    }

    return thiz;
}

void packet_transfer_destroy(PacketTransfer *thiz)
{
    if (thiz != NULL)
    {
        if (thiz->uploader_socket != -1)
        {
            close(thiz->uploader_socket);
        }

        int i;
        for (i = 0; i < NODE_NUM; i++)
        {
            dlist_destroy(thiz->command_set[i]);
        }

        free(thiz);
    }
}

Ret packet_transfer_process(PacketTransfer *thiz, int socket)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(socket != -1, RET_INVALID_PARAMS);

    thiz->uploader_socket = socket;

    for (;;)
    {
        /* 以一条命令为单位，接收命令，读取命令，直到上位机关闭连接才退出处理流程 */
        if (process_one_command(thiz) != RET_OK)
        {
            break;
        }
    }

    /* 在退出处理流程后，关闭连接 */
    close(thiz->uploader_socket);
    thiz->uploader_socket = -1;

    return RET_OK;
}

Ret packet_transfer_reset(PacketTransfer *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    int i;
    int j;
    int size;

    /* 删除命令集合中的所有命令 */
    for (i = 0; i < NODE_NUM; i++)
    {
        size = dlist_length(thiz->command_set[i]);
        for (j = 0; i < size; j++)
        {
            dlist_delete(thiz->command_set[i], j);
        }
    }

    thiz->uploader_socket = -1;

    return RET_OK;
}
