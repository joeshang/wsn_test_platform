/**
 * @file oint_protocol.h
 * @brief 定义通信协议中各个字段的索引，以及控制命令
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#ifndef __OINT_PROTOCOL_H__
#define __OINT_PROTOCOL_H__

/*
 * OINT的数据通信协议分为3层：
 * 1. Node层：定义传感器节点与FPGA之间的数据通信格式（使用串口）
 * 2. Board层：定义FPGA与ARM之间的数据通信格式（使用SPI）
 * 3. Server层：定义ARM与上位机之间的数据通信格式（使用TCP）
 */

/******************************************************************
                              Node层                             
 从低到高格式如下：
 - Delimiter：1字节，0x7E
 - Direction：1字节，0x44表示从FPGA到节点，0x45表示从节点到FPGA
 - Empty：1字节，始终为0
 - DestinationNodeID：2字节，表示目标地址节点号，小端格式 
 - SourceNodeID：2字节，表示源地址节点号，小端格式
 - PayloadLength：1字节
 - GroupID：1字节，一般为0
 - AMTypeID：1字节，AM类型号
 - Payload：变长，
   - 第一个字节为Type，描述Payload的类型
   - 剩余的字节为Data，Type + Data为PayloadLength
 - CRC：2字节，大端格式
 - Delimiter：1字节，0x7E
 ******************************************************************/
#define NODE_DELIMITER_INDEX                    0
#define NODE_DIRECTION_INDEX                    1
#define NODE_DST_NODE_ID_INDEX                  3
#define NODE_SRC_NODE_ID_INDEX                  5
#define NODE_PAYLOAD_LEN_INDEX                  7
#define NODE_PAYLOAD_TYPE_INDEX                 10
#define NODE_PAYLOAD_DATA_INDEX                 11
#define NODE_PAYLOAD_TYPE_WIDTH                 1
#define NODE_CRC_WIDTH                          2
#define NODE_PACKET_MAX_SIZE                    256

#define NODE_DIRECTION_TO_NODE                  0x44
#define NODE_DIRECTION_FROM_NODE                0x45

#define NODE_DELIMITER                          0x7E
#define NODE_ESCAPE                             0x7D
#define node_delimiter_transfer(c)              ((c) - 0x20)    /* 0x7E -> 0x5E, 0x7D -> 0x5D */

#define node_type_convert_resp_to_cmd(resp)     ((resp) + 0x20)
/* 上报数据的类型 */
#define NODE_PAYLOAD_TYPE_RESP_RECV             0x11    /* 抄送无线接收分组信息 */
#define NODE_PAYLOAD_TYPE_RESP_SEND             0x12    /* 抄送无线发送分组信息 */
#define NODE_PAYLOAD_TYPE_RESP_NODE_ID          0x13    /* 上报节点号 */
#define NODE_PAYLOAD_TYPE_RESP_CHANNEL          0x14    /* 上报信道 */
#define NODE_PAYLOAD_TYPE_RESP_RF_POWER         0x15    /* 上报射频功率 */
#define NODE_PAYLOAD_TYPE_RESP_VENDOR           0x16    /* 上报生产厂商 */
#define NODE_PAYLOAD_TYPE_RESP_SENSOR_CNT       0x17    /* 上报传感器数量 */
#define NODE_PAYLOAD_TYPE_RESP_NODE_INFO        0x18    /* 上报节点标识 */
#define NODE_PAYLOAD_TYPE_RESP_CURRENT          0x19    /* 上报电流值 */
#define NODE_PAYLOAD_TYPE_RESP_RANGE            0x2F    /* 上报支持的测试范围 */
/* 查询和设置命令的类型 */
#define NODE_PAYLOAD_TYPE_CMD_RECV              0x31    /* 设置抄送无线接收分组信息 */
#define NODE_PAYLOAD_TYPE_CMD_SEND              0x32    /* 设置抄送无线发送分组信息 */
#define NODE_PAYLOAD_TYPE_CMD_NODE_ID           0x33    /* 查询节点号 */
#define NODE_PAYLOAD_TYPE_CMD_CHANNEL           0x34    /* 查询信道 */
#define NODE_PAYLOAD_TYPE_CMD_RF_POWER          0x35    /* 查询射频功率 */
#define NODE_PAYLOAD_TYPE_CMD_VENDOR            0x36    /* 查询生产厂商 */
#define NODE_PAYLOAD_TYPE_CMD_SENSOR_CNT        0x37    /* 查询传感器数量 */
#define NODE_PAYLOAD_TYPE_CMD_NODE_INFO         0x38    /* 查询节点标识 */
#define NODE_PAYLOAD_TYPE_CMD_CURRENT           0x39    /* 查询节点电流值 */
#define NODE_PAYLOAD_TYPE_CMD_RANGE             0x4F    /* 查询支持的测试范围 */
/* 控制命令的类型 */
#define NODE_PAYLOAD_TYPE_CMD_MODIFY_CHANNEL    0x51    /* 修改信道 */
#define NODE_PAYLOAD_TYPE_CMD_MODIFY_POWER      0x52    /* 修改功率 */
#define NODE_PAYLOAD_TYPE_CMD_REMOTE_PROG       0x53    /* 远程编程 */
#define NODE_PAYLOAD_TYPE_CMD_POWER_SWITCH      0x54    /* 开关节点 */


/******************************************************************
                             Board层
 Board层命令在Node层命令包前加入PortID字段，表明要发送给哪个节点，
 格式如下：
                  < PortID(1B) | NodePacket >
 但是，有几种情况是直接跟FPGA交互，没有NodePacket，只有一个字节，
 包括重编程跟对节点的开断电，格式为：
                        < CommandType(1B) >
 Board层回应为标准16个字节长度，FPGA从Node层得到的回应包如果一个包
 放不下，则会拆成多个16字节包。有两种类型回应包：节点数据包跟电流包，
 格式如下：
   < PortID_DataLength(1B) | TimeStamp(4B) | Data(10B) | Even(1B) >
 PortID_DataLength中高4位为PortID，低4位为DataLength
 对于节点数据包而言：
   DataLength为数据长度，Data不够10B的填充0x0
 对于电流数据包而言：
   DataLength为固定的0xF，Data中前2字节为电流数据，其余填充0x11，
 奇偶校验为固定的0x11

 TIPS:
   在Board层，使用的是PortID（范围是0-7），而不是NodeID（范围是1-8）
 因为对于FPGA而言，对应其0-7的端口（Port），其他两层均为NodeID
 ******************************************************************/
#define CMD_B_PACKET_MAX_SIZE                   50
#define CMD_B_HEADER_LEN                        1
#define CMD_B_PORT_ID_INDEX                     0
#define CMD_B_PAYLOAD_TYPE_INDEX                (NODE_PAYLOAD_TYPE_INDEX + CMD_B_HEADER_LEN)

/* 直接控制FPGA的命令格式为：Base + PortID(NodeID - 1) */
#define get_board_cmd_with_port_id(base, id)    ((base) + (id))
#define CMD_B_REPROG_START_BASE                 0x10    /* 开始重编程 */
#define CMD_B_REPROG_STOP_BASE                  0x20    /* 停止重编程 */
#define CMD_B_NODE_OPEN_BASE                    0x30    /* 打开节点 */
#define CMD_B_NODE_CLOSE_BASE                   0x40    /* 关闭节点 */

#define RESP_B_PACKET_MAX_SIZE                  16      /* 从FPGA上来的包大小固定为16字节 */
#define RESP_B_GET_PORT_ID(header)              ((header) >> 4) & 0xF
#define RESP_B_GET_DATA_LEN(header)             (header) & 0xF
#define RESP_B_DATA_LEN_MAX                     10      /* 在16字节包中DataLength最多为10个字节 */

#define RESP_B_HEADER_LEN                       5
#define RESP_B_PORTID_DATALEN_INDEX             0
#define RESP_B_TIME_STAMP_INDEX                 1
#define RESP_B_TIME_STAMP_WIDTH                 4
#define RESP_B_CURRENT_INDEX                    RESP_B_HEADER_LEN
#define RESP_B_CURRENT_WIDTH                    2
#define RESP_B_TYPE_INDEX                       RESP_B_HEADER_LEN + NODE_PAYLOAD_TYPE_INDEX
#define RESP_B_EVEN_CHECK_INDEX                 (RESP_B_PACKET_MAX_SIZE - 1)

#define RESP_B_PACKET_TYPE_INVALID              0xFF
#define RESP_B_PACKET_TYPE_CURRENT              0xF

/******************************************************************
                             Server层
 Server层命令只是简单的在Board层的数据包前加入表示Board层数据包的
 长度字段，格式如下：
              < PacketLength(2B) | BoardPacket >
 Server层回应在Node层回应包前加入包长度字段，NodeID字段，TimeStamp
 +Data长度字段，TimeStamp字段，在Node层回应包尾部加入CRC字段，格式
 如下：
    < PacketLength(2B) | NodeID(1B) | TimeStampAndNodeLength(2B) |
            TimeStamp(4B) | NodePacket | CRC(2B) >
 ******************************************************************/
#define CMD_S_HEADER_LEN                        2

#define RESP_S_HEADER_LEN                       9
#define RESP_S_PACKET_MAX_SIZE                  (NODE_PACKET_MAX_SIZE + RESP_S_HEADER_LEN)
#define RESP_S_PACKET_LEN_WIDTH                 2
#define RESP_S_NODE_ID_INDEX                    2
#define RESP_S_NODE_ID_WIDTH                    1
#define RESP_S_TIME_NODE_LEN_INDEX              3
#define RESP_S_TIME_NODE_LEN_WIDTH              2
#define RESP_S_TIMESTAMP_INDEX                  5
#define RESP_S_TIMESTAMP_WIDTH                  4
#define RESP_S_CRC_WIDTH                        2
#define RESP_S_TYPE_INDEX                       (NODE_PAYLOAD_TYPE_INDEX + RESP_B_HEADER_LEN)

/*
 * OINT的重编程协议
 */
/* 格式为：< NodeID(1B) | PrefixString(6B) | HexFileLength(4B) | HexFileContent > */
#define REPROG_CMD_HEADER_LEN                   11
#define REPROG_CMD_NODE_ID_INDEX                0
#define REPROG_CMD_PREFIX_STR_INDEX             1
#define REPROG_CMD_HEX_FILE_LEN_INDEX           7
#define REPROG_CMD_HEX_FILE_LEN_WIDTH           4
#define REPROG_CMD_PREFIX_STR                   "PROGV3"

#endif
