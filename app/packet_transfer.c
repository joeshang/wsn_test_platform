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

enum _PacketState
{
    IN_PACKET,
    OUT_PACKET
};

typedef struct _Command
{
    uint8_t port_id;
    uint8_t type;
    uint16_t to_board_packet_len;
    uint8_t to_board_packet[CMD_B_PACKET_MAX_SIZE];
}Command;

typedef struct _Response
{
    uint8_t port_id;
    uint8_t type;
    uint8_t time_stamp[RESP_B_TIME_STAMP_WIDTH];
    uint16_t node_packet_len;
    uint8_t node_packet[NODE_PACKET_MAX_SIZE];
    uint8_t is_finished;
}Response;

struct _PacketTransfer
{
    int uploader_socket;    // 上位机连接socket(TCP)
    /*int display_socket;     // 本机Qt显示socket(UDP)*/
    DList *command_set[NODE_NUM];
    Response response;
};

static Ret  process_one_command(PacketTransfer *thiz);
static void handle_command(PacketTransfer *thiz, Command *command);
static Ret  process_one_response(PacketTransfer *thiz);
static void handle_response(PacketTransfer *thiz);
static void send_response(PacketTransfer *thiz);

static void handle_command(PacketTransfer *thiz, Command *command)
{
    uint8_t payload_data;

    if (command->type == NODE_PAYLOAD_TYPE_CMD_POWER_SWITCH)
    {
        uint8_t to_board;
        payload_data = command->to_board_packet[CMD_B_TYPE_INDEX + 1];

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
        // TODO: gather_board_write(board, &command->to_board_packet, command->to_board_packet_len);
        
        int wait_for_response = TRUE;
        switch (command->type)
        {
            case NODE_PAYLOAD_TYPE_CMD_CURRENT:
            case NODE_PAYLOAD_TYPE_CMD_POWER_SWITCH:
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

    Command *command = (Command *)malloc(sizeof(Command));
    if (command == NULL)
    {
        fprintf(stderr, "malloc command failed\n");
        goto fail;
    }

    /* 1. 读取命令包中发送至FPGA包的长度 */ 
    n = read(thiz->uploader_socket, &command->to_board_packet_len, CMD_S_HEADER_LEN);
    if (n == -1)
    {
        fprintf(stderr, "data uploader read command to_board_packet_len failed\n");
        goto fail;
    }
    else if (n == 0)
    {
        fprintf(stderr, "peer client close connection\n");
        goto fail;
    }
    command->to_board_packet_len = ntohs(command->to_board_packet_len);

    /* 2. 读取命令包中发送至FPGA板的包 */
    n = read(thiz->uploader_socket, command->to_board_packet, command->to_board_packet_len);
    if (n == -1)
    {
        fprintf(stderr, "data uploader read command to_board_packet failed\n");
        goto fail;
    }
    else if (n == 0)
    {
        fprintf(stderr, "peer client close connection\n");
        goto fail;
    }

    /* 3. 根据从上位机接收的命令包初始化Command */
    command->port_id = command->to_board_packet[CMD_B_PORT_ID_INDEX];
    command->type = command->to_board_packet[CMD_B_TYPE_INDEX];

    debug("[PacketTransfer]: receive command -> ");
#ifdef _DEBUG
    print_hex(command->to_board_packet, command->to_board_packet_len, stdout);
#endif

    handle_command(thiz, command);        

    return RET_OK;
fail:
    SAFE_FREE(command);
    return RET_FAIL;
}

static int response_find_cb(void *ctx, void *data)
{
    Command *command = (Command *)data;
    uint8_t *type = (uint8_t *)ctx;

    return (command->type == *type);
}

static void handle_response(PacketTransfer *thiz)
{
    /* 能够响应上位机命令，则点亮节点对应的红灯 */
    if (thiz->response.type == NODE_PAYLOAD_TYPE_RESP_NODE_ID)
    {
        // TODO: gather_board_led_red(gather_board, thiz->response.port_id, LED_ON);
    }

    /* 节点上报数据包，则节点对应的绿灯闪烁一次 */
    // TODO: gather_board_led_green(gather_board, thiz->response.port_id, LED_ON);
    
    /* 在命令集合中找到回应对应的命令项并删除 */
    DList *command_list = thiz->command_set[thiz->response.port_id - 1];
    int command_type = node_type_convert_resp_to_cmd(thiz->response.type);
    int target_index = dlist_find(command_list, response_find_cb, &command_type);
    dlist_delete(command_list, target_index);
}

static void send_response(PacketTransfer *thiz)
{
    uint16_t packet_len = RESP_S_HEADER_LEN + thiz->response.node_packet_len + RESP_S_CRC_WIDTH;
    uint8_t *packet_buf = (uint8_t *)malloc(packet_len);
    
    /* 1. 将包长度加入缓冲区，大端格式 */
    uint16_t packet_len_send = htons(packet_len);
    memcpy(packet_buf, 
           &packet_len_send, 
           RESP_S_PACKET_LEN_WIDTH);

    /* 2. 将节点号加入缓冲区 */
    packet_buf[RESP_S_PORT_ID_INDEX] = thiz->response.port_id;

    /* 3. 将时间戳+数据长度加入缓冲区 */
    uint16_t timestamp_data_len = RESP_S_TIMESTAMP_WIDTH + thiz->response.node_packet_len;
    timestamp_data_len = htons(timestamp_data_len);
    memcpy(packet_buf + RESP_S_TIME_NODE_LEN_INDEX, 
           &timestamp_data_len, 
           RESP_S_TIME_NODE_LEN_WIDTH);

    /* 4. 将时间戳加入缓冲区，大端格式 */
    memcpy(packet_buf + RESP_S_TIMESTAMP_INDEX, 
           thiz->response.time_stamp, 
           RESP_S_TIMESTAMP_WIDTH);

    /* 5. 将Node包加入缓冲区 */
    memcpy(packet_buf + RESP_S_HEADER_LEN, 
           thiz->response.node_packet, 
           thiz->response.node_packet_len);

    /* 6. 计算2-5的CRC，加入缓冲区，大端格式 */
    int crc_index = RESP_S_HEADER_LEN + thiz->response.node_packet_len;
    uint8_t crc = 0;
    packet_buf[crc_index] = crc;

    debug("[PacketTransfer]: send response -> ");
#ifdef _DEBUG
    print_hex(packet_buf, packet_len, stdout);
#endif

    /* 发送回应 */
    write(thiz->uploader_socket, packet_buf, packet_len);

    SAFE_FREE(packet_buf);
}

static Ret  process_one_response(PacketTransfer *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    uint8_t from_board[RESP_B_PACKET_MAX_SIZE];

    /* 1. 从FPGA板读取16字节（因为FPGA发给ARM的数据就是16字节长的包）*/
    // TODO: gather_board_read(board, &from_board, RESP_B_PACKET_MAX_SIZE);

    debug("[PacketTransfer]: receive response -> ");
#ifdef _DEBUG
    print_hex(from_board, RESP_B_PACKET_MAX_SIZE, stdout);
#endif
    
    /* 2. 对读取的包进行奇偶校验 */
    if (even_check(from_board, RESP_B_PACKET_MAX_SIZE) == FALSE)
    {
        debug("[PacketTransfer]: even check failed\n");
        /* 检验没有通过，退出处理流程 */
        return RET_FAIL;
    }

    /* 3. 根据第一个字节判断是什么类型的包 */
    uint8_t header = from_board[RESP_B_PORTID_DATALEN_INDEX];
    uint8_t data_len = RESP_B_GET_DATA_LEN(header);

    /* 全是0xFF的包跟数据长度超出正常范围的包都是无效包，不处理，直接退出流程 */
    if (header == RESP_B_PACKET_TYPE_INVALID
        || (data_len > RESP_B_DATA_LEN_MAX && data_len != RESP_B_PACKET_TYPE_CURRENT))
    {
        debug("[PacketTransfer]: invalid packet type\n");
        return RET_FAIL;
    }


    /* 电流包 */
    if (data_len == RESP_B_PACKET_TYPE_CURRENT)
    {
        debug("[PacketTransfer]: current packet\n");
        thiz->response.port_id = RESP_B_GET_PORT_ID(header);
        memcpy(&thiz->response.time_stamp,
                from_board + RESP_B_TIME_STAMP_INDEX,
                RESP_B_TIME_STAMP_WIDTH);

        /* 生成规范的电流包格式（与数据包格式相同）*/
        memset(thiz->response.node_packet, 0, thiz->response.node_packet_len);
        /* 1. 分隔符（头部） */
        thiz->response.node_packet[NODE_DELIMITER_INDEX] = NODE_DELIMITER;
        /* 2. 数据方向 */
        thiz->response.node_packet[NODE_DIRECTION_INDEX] = NODE_DIRECTION_FROM_NODE;
        /* 3. 节点号，放在SourcePortID处，表明是哪个节点的电流包 */
        thiz->response.node_packet[NODE_SRC_NODE_ID_INDEX] = thiz->response.port_id;
        /* 4. Payload长度 */
        thiz->response.node_packet[NODE_PAYLOAD_LEN_INDEX] = NODE_PAYLOAD_TYPE_WIDTH +  RESP_B_CURRENT_WIDTH;
        /* 5. Payload中类型设为电流包 */
        thiz->response.node_packet[NODE_PAYLOAD_TYPE_INDEX] = NODE_PAYLOAD_TYPE_RESP_CURRENT;
        /* 6. Payload中数据从FPGA发送过来的包中读取 */
        int copy_count;
        int index = NODE_PAYLOAD_DATA_INDEX;
        copy_count = copy_with_process_delimiter(thiz->response.node_packet + index,
                from_board + RESP_B_CURRENT_INDEX,
                RESP_B_CURRENT_WIDTH);
        index += copy_count;
        /* 7. 计算CRC */
        // XXX: 生成规范格式的电流包需要计算CRC吗？（Node的CRC主要用于节点与FPGA通信间的验证）
        uint16_t crc = crc_16_width(thiz->response.node_packet + NODE_DELIMITER_INDEX, index);
        memcpy(thiz->response.node_packet + index, &crc, NODE_CRC_WIDTH);
        index += NODE_CRC_WIDTH;
        /* 8. 分隔符（尾部） */
        thiz->response.node_packet[index++] = NODE_DELIMITER;

        thiz->response.node_packet_len = index;
        thiz->response.is_finished = TRUE;
    }
    /* 数据包 */
    else
    {
        static int packet_state = OUT_PACKET;

        debug("[PacketTransfer]: data packet\n");

        /* 利用状态机处理被FPGA拆包的数据包 */
        if (packet_state == OUT_PACKET)
        {
            uint8_t delimiter = from_board[RESP_B_HEADER_LEN + NODE_DELIMITER_INDEX];
            uint8_t direction = from_board[RESP_B_HEADER_LEN + NODE_DIRECTION_INDEX];

            /* Node数据包至少被分成2个包，所以Node数据包头部所在的包一定占满整个FPGA包 */
            if ((delimiter == NODE_DELIMITER) 
                && (direction == NODE_DIRECTION_FROM_NODE)
                && (data_len == RESP_B_DATA_LEN_MAX))   
            {
                packet_state = IN_PACKET;

                /* 重置response */
                memset(&thiz->response, 0, sizeof(thiz->response));
                thiz->response.is_finished = FALSE;

                thiz->response.port_id = RESP_B_GET_PORT_ID(header);
                thiz->response.type = from_board[RESP_B_TYPE_INDEX];
                memcpy(&thiz->response.time_stamp,
                        from_board + RESP_B_TIME_STAMP_INDEX,
                        RESP_B_TIME_STAMP_WIDTH);
                memcpy(thiz->response.node_packet, 
                        from_board + RESP_B_HEADER_LEN, 
                        data_len);
                thiz->response.node_packet_len += data_len;
            }
            else
            {
                /* 在OUT_PACKET状态下，非Node数据包头部的包均认为是无效包，被丢弃 */
                return RET_FAIL;
            }
        }
        else if (packet_state == IN_PACKET)
        {
            memcpy(thiz->response.node_packet + thiz->response.node_packet_len,
                    from_board + RESP_B_HEADER_LEN, 
                    data_len);
            thiz->response.node_packet_len += data_len;

            uint8_t last_byte_of_data = from_board[RESP_B_EVEN_CHECK_INDEX - 1];
            /* 从FPGA读取的包中数据没有被Node数据包占满或者最后一位为分隔符，则认为得到一个完整的Node数据包*/
            if (data_len != RESP_B_DATA_LEN_MAX || last_byte_of_data == NODE_DELIMITER)
            {
                packet_state = OUT_PACKET;
                thiz->response.is_finished = TRUE;

                handle_response(thiz);

                return RET_OK;
            }
        }
    }

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

        memset(&thiz->response, 0, sizeof(thiz->response));
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

    // TODO: using select or creating a new thread to reading data from board
    /*for (;;)                                     */
    /*{                                            */
    /*    if (process_one_response(thiz) != RET_OK)*/
    /*    {                                        */
    /*        [> 重置Response状态 <]           */
    /*        thiz->response.is_finished = FALSE;  */
    /*    }                                        */

    /*    if (thiz->response.is_finished == TRUE)  */
    /*    {                                        */
    /*        send_response(thiz);                 */

    /*        [> 重置Response状态 <]           */
    /*        thiz->response.is_finished = FALSE;  */
    /*    }                                        */
    /*}                                            */

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

    memset(&thiz->response, 0, sizeof(thiz->response));

    if (thiz->uploader_socket != -1)
    {
        close(thiz->uploader_socket);
    }
    thiz->uploader_socket = -1;

    return RET_OK;
}
