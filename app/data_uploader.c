/**
 * @file data_uploader.h
 * @brief 负责接收并解析上位机发送命令，发送给FPGA板，并从FPGA板读取数据，发回上位机
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "data_uploader.h"
#include "oint_protocol.h"
#include "util.h"

#define CMD_B_BUFFER_SIZE               40

typedef struct _Command
{
    int port_id;
    int type;
    int to_board_cmd_len;
    unsigned char to_board_cmd[CMD_B_BUFFER_SIZE];
}Command;

struct _DataUploader
{
    int uploader_socket;    // 上位机连接socket(TCP)
    /*int display_socket;     // 本机Qt显示socket(UDP)*/
};

DataUploader *data_uploader_create()
{
    DataUploader *thiz = (DataUploader *)malloc(sizeof(DataUploader));

    if (thiz != NULL)
    {
        thiz->uploader_socket = -1;
    }

    return thiz;
}

void data_uploader_destroy(DataUploader *thiz)
{
    if (thiz != NULL)
    {
        if (thiz->uploader_socket != -1)
        {
            close(thiz->uploader_socket);
        }

        free(thiz);
    }
}

Ret data_uploader_process(DataUploader *thiz, int socket)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(socket != -1, RET_INVALID_PARAMS);

    thiz->uploader_socket = socket;

    int n;
    uint16_t command_size = 0;
    Command *command = NULL;

    /* 
     * 处理流程就是：
     * 1. 等待上位机发送命令
     * 2. 处理命令
     * 3. 回到1，直到上位机关闭连接，退出处理流程
     */
    command = (Command *)malloc(sizeof(Command));
    for (;;)
    {
        if (command == NULL)
        {
            fprintf(stderr, "malloc command failed\n");
            break;
        }

        /* 1. 读取命令包的长度跟PortID */ 
        read(thiz->uploader_socket, &command_size, CMD_U_LENGTH_WIDTH);
        read(thiz->uploader_socket, &command->port_id, CMD_U_PORT_ID_WIDTH);
        command_size = ntohs(command_size);
        command->to_board_cmd_len = command_size - 1;

        /* 2. 读取命令包中发送至FPGA板的命令 */
        n = read(thiz->uploader_socket, command->to_board_cmd, command->to_board_cmd_len);
        if (n == -1)
        {
            fprintf(stderr, "data uploader read command content failed\n");
            break;
        }
        else if (n == 0)
        {
            fprintf(stderr, "peer client close connection\n");
            break;
        }

        /* 3. 根据从上位机接收的命令包初始化Command */
        command->type = command->to_board_cmd[CMD_B_TYPE_INDEX];

        debug("[DataUploader]: receive command -> ");
#ifdef _DEBUG
        print_hex(command->to_board_cmd, command->to_board_cmd_len, stdout);
#endif
    }

    SAFE_FREE(command);
    close(thiz->uploader_socket);
    thiz->uploader_socket = -1;

    return RET_FAIL;
}

Ret data_uploader_reset(DataUploader *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    return RET_OK;
}
