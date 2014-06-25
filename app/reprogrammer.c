/**
 * @file reprogramer.h
 * @brief 负责处理重编程流程
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-14
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "reprogrammer.h"
#include "oint_protocol.h"

#define HEX_FILE_RECV_PATH          "program.hex"
#define RECV_BUF_SIZE               4096

enum _ReprogStatus
{
    REPROG_STATUS_SUCCESS,
    REPROG_STATUS_RECV_CMD_HEX_FAILED,
    REPROG_STATUS_INIT_BAUDRATE_FAILED,
    REPROG_STATUS_MASS_ERASE_FAILED,
    REPROG_STATUS_RX_PASSWORD_FAILED,
    REPROG_STATUS_TX_DATA_FAILED,
    REPROG_STATUS_MODIFY_BAUDRATE_FAILED,
};

struct _Reprogrammer
{
    FILE *hex_fd;
    int server_socket;
    GatherBoard *gather_board;
};

static Ret hex_line_data_cb(int addr, const unsigned char *buf, int length, void *ctx)
{

}

static Ret reprogrammer_node_program_setup(Reprogrammer *thiz)
{
    return_val_if_fail(thiz != thiz, RET_INVALID_PARAMS);
    Ret ret = RET_OK;

    /* 1. 将ARM6板波特率初始化为BSL默认值 */

    /* 2. 擦除节点的flash */

    /* 3. 在编程之前先向节点发送密码进行鉴权 */

    /* 4. 将节点波特率变成更高值，加快重编程速度 */

    return ret;
}

static Ret reprogrammer_node_program_doing(Reprogrammer *thiz)
{
    return_val_if_fail(thiz != thiz, RET_INVALID_PARAMS);
    Ret ret = RET_OK;

    HexParser *hex_parser = hex_parser_create(hex_line_data_cb, thiz);
    if (hex_parser == NULL)
    {
        fprintf(stderr, "create hex parser failed\n");
        return RET_FAIL;
    }

    /* 5. 解析Hex文件，在回调函数中按行写入并验证 */
    hex_parser_process_file(hex_parser, thiz->hex_fd);

    hex_parser_destroy(hex_parser);

    return ret;
}

static ret reprogrammer_node_program_end(reprogrammer *thiz)
{
    return_val_if_fail(thiz != thiz, RET_INVALID_PARAMS);
    Ret ret = RET_OK;

    return ret;
}

static Ret reprogrammer_receive_hex_file(Reprogrammer *thiz, int hex_file_len)
{
    unsigned char recv_buf[RECV_BUF_SIZE];
    int readn = 0;
    
    thiz->hex_fd = fopen(HEX_FILE_RECV_PATH, "w+");
    if (thiz->hex_fd == NULL)
    {
        fprintf(stderr, "create %s file failed\n", HEX_FILE_RECV_PATH);
        return RET_FAIL;
    }

    while (hex_file_len > 0)
    {
        readn = read(thiz->server_socket, recv_buf, RECV_BUF_SIZE);
        if (readn == -1)
        {
            fprintf(stderr, "reprogrammer read hex file failed\n");
            return RET_FAIL;
        }
        else if (readn == 0)
        {
            fprintf(stderr, "peer client close connection\n");
            return RET_FAIL;
        }

        write(thiz->hex_fd, recv_buf, readn);
        hex_file_len -= readn;
    }

    return RET_OK;
}

Reprogrammer *reprogrammer_create(GatherBoard *gather_board)
{
    Reprogrammer *thiz = (Reprogrammer *)malloc(sizeof(Reprogrammer));

    if (thiz != NULL)
    {
        thiz->hex_fd = -1;
        thiz->server_socket = -1;
        thiz->gather_board = gather_board;
    }

    return thiz;
}

void reprogrammer_destroy(Reprogrammer *thiz)
{
    if (thiz != NULL)
    {
        if (thiz->server_socket != -1)
        {
            close(thiz->server_socket);
            thiz->server_socket = -1;
        }

        if (thiz->hex_fd != -1)
        {
            close(thiz->hex_fd);
            thiz->hex_fd = -1;
        }

        free(thiz);
    }
}

Ret  reprogrammer_process(Reprogrammer *thiz, int socket)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    
    uint32_t hex_file_len;
    uint8_t node_id;
    uint8_t *prefix_string;
    uint8_t cmd_header[REPROG_CMD_HEADER_LEN];
    int reprog_state = REPROG_STATUS_SUCCESS;
    
    thiz->server_socket = socket;
    /* 1. 读取重编程命令 */
    if (read(thiz->server_socket, cmd_header, REPROG_CMD_HEADER_LEN) == -1)
    {
        fprintf(stderr, "reprogrammer read command failed\n");
        reprog_state = REPROG_STATUS_RECV_CMD_HEX_FAILED;
        goto exit;
    }

    /* 2. 解析重编程命令 */
    node_id = cmd_header[REPROG_CMD_NODE_ID_INDEX];
    memcpy(&hex_file_len, cmd_header + REPROG_CMD_HEX_FILE_LEN_INDEX, REPROG_CMD_HEX_FILE_LEN_WIDTH);
    hex_file_len = ntohl(hex_file_len); /* 大端转小端 */
    prefix_string = cmd_header + REPROG_CMD_PREFIX_STR_INDEX;
    cmd_header[REPROG_CMD_HEX_FILE_LEN_INDEX] = '\0'; 

    debug("[Reprogrammer]: receive command -> %s node %d with hex file length %d\n",
            prefix_string, node_id, hex_file_len);

    /* 3. 向FPGA板发送开始重编程命令 */
    uint8_t to_board = get_board_cmd_with_port_id(CMD_B_REPROG_START_BASE, node_id - 1);
    // TODO: gather_board_write(thiz->gather_board, &to_board, 1);
    // usleep(1000);

    /* 4. 检查命令的前导符是否正确 */
    if (strncmp(prefix_string, REPROG_CMD_PREFIX_STR, strlen(REPROG_CMD_PREFIX_STR)) != 0)
    {
        fprintf(stderr, "reprogrammer read invalid prefix string: %s\n", prefix_string);
        reprog_state = REPROG_STATUS_RECV_CMD_HEX_FAILED;
        goto exit;
    }

    /* 5. 接收重编程的Hex文件 */    
    if (reprogrammer_receive_hex_file(thiz, hex_file_len) != RET_OK)
    {
        reprog_state = REPROG_STATUS_RECV_CMD_HEX_FAILED;
        goto exit;
    }

    /* 6. 对节点进行重编程 */
    reprogrammer_node_program_setup(thiz);
    reprogrammer_node_program_doing(thiz);
    reprogrammer_node_program_end(thiz);
    
exit:
    if (thiz->server_socket != -1)
    {
        close(thiz->server_socket);
        thiz->server_socket = -1;
    }

    if (thiz->hex_fd != -1)
    {
        close(thiz->hex_fd);
        thiz->hex_fd = -1;
    }

    return RET_OK;
}
