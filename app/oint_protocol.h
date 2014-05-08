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
 - Empty：2字节，始终为0
 - SourceNodeID：2字节，表示目标地址节点号，小端格式 
 - DestinationNodeID：2字节，表示源地址节点号，小端格式
 - PayloadLength：1字节
 - GroupID：1字节，一般为0
 - AMTypeID：1字节，AM类型号
 - Payload：变长，其中第一个字节为type，描述Payload的类型
 - CRC：2字节，大端格式
 - Delimiter：1字节，0x7E
 ******************************************************************/
#define NODE_SRC_NODE_ID_INDEX              4
#define NODE_DST_NODE_ID_INDEX              6
#define NODE_PAYLOAD_LEN_INDEX              8
#define NODE_TYPE_INDEX                     11
/* 上报数据的类型 */
#define PAYLOAD_TYPE_UPLOAD_RECV            0x11    /* 抄送无线接收分组信息 */
#define PAYLOAD_TYPE_UPLOAD_SEND            0x12    /* 抄送无线发送分组信息 */
#define PAYLOAD_TYPE_UPLOAD_NODE_NUM        0x13    /* 上报节点号 */
#define PAYLOAD_TYPE_UPLOAD_CHANNEL         0x14    /* 上报信道 */
#define PAYLOAD_TYPE_UPLOAD_RF_POWER        0x15    /* 上报射频功率 */
#define PAYLOAD_TYPE_UPLOAD_VENDOR          0x16    /* 上报生产厂商 */
#define PAYLOAD_TYPE_UPLOAD_SENSOR_CNT      0x17    /* 上报传感器数量 */
#define PAYLOAD_TYPE_UPLOAD_NODE_ID         0x18    /* 上报节点标识 */
#define PAYLOAD_TYPE_UPLOAD_CURRENT         0x19    /* 上报电流值 */
#define PAYLOAD_TYPE_UPLOAD_RANGE           0x2F    /* 上报支持的测试范围 */
/* 查询和设置命令的类型 */
#define PAYLOAD_TYPE_QUERY_RECV             0x31    /* 设置抄送无线接收分组信息 */
#define PAYLOAD_TYPE_QUERY_SEND             0x32    /* 设置抄送无线发送分组信息 */
#define PAYLOAD_TYPE_QUERY_NODE_NUM         0x33    /* 查询节点号 */
#define PAYLOAD_TYPE_QUERY_CHANNEL          0x34    /* 查询信道 */
#define PAYLOAD_TYPE_QUERY_RF_POWER         0x35    /* 查询射频功率 */
#define PAYLOAD_TYPE_QUERY_VENDOR           0x36    /* 查询生产厂商 */
#define PAYLOAD_TYPE_QUERY_SENSOR_CNT       0x37    /* 查询传感器数量 */
#define PAYLOAD_TYPE_QUERY_NODE_ID          0x38    /* 查询节点标识 */
#define PAYLOAD_TYPE_QUERY_CURRENT          0x39    /* 查询节点电流值 */
#define PAYLOAD_TYPE_QUERY_RANGE            0x4F    /* 查询支持的测试范围 */
/* 控制命令的类型 */
#define PAYLOAD_TYPE_CNTL_MODIFY_CHANNEL    0x51    /* 修改信道 */
#define PAYLOAD_TYPE_CNTL_MODIFY_POWER      0x52    /* 修改功率 */
#define PAYLOAD_TYPE_CNTL_REMOTE_PROG       0x53    /* 远程编程 */
#define PAYLOAD_TYPE_CNTL_POWER_SWITCH      0x54    /* 开关节点 */


/******************************************************************
                             Board层
 Board层命令在Node层命令包前加入PortID字段，表明要发送给哪个节点，
 格式如下：
                  < PortID(1B) | NodePacket >
 但是，有几种情况是直接跟FPGA交互，没有NodePacket，只有一个字节，
 包括重编程跟对节点的开断电，格式为：
                        < CommandType(1B) >
 Board层回应在Node层回应包前加入PortID字段，TimeStamp+Data长度字段，
 TimeStamp字段，在Node层回应包尾部加入CRC字段，格式如下：
          < PortID(1B) | TimeStampAndNodeLength(2B) |
            TimeStamp(4B) | NodePacket | CRC(2B) >
 ******************************************************************/
#define CMD_B_PACKET_MAX_SIZE               50
#define CMD_B_HEADER_LEN                    1
#define CMD_B_PORT_ID_INDEX                 0
#define CMD_B_TYPE_INDEX                    (NODE_TYPE_INDEX + CMD_B_HEADER_LEN)

/* 直接控制FPGA的命令格式为：Base + NodeID - 1 */
#define CMD_B_REPROG_START_BASE             0x10    /* 开始重编程 */
#define CMD_B_REPROG_STOP_BASE              0x20    /* 停止重编程 */
#define CMD_B_NODE_OPEN_BASE                0x30    /* 打开节点 */
#define CMD_B_NODE_CLOSE_BASE               0x40    /* 关闭节点 */

#define RESP_B_PACKET_MAX_SIZE              512
#define RESP_B_HEADER_LEN                   7
#define RESP_B_PORT_ID_WIDTH                1
#define RESP_B_TIME_NODE_LEN_WIDTH          2
#define RESP_B_CRC_WIDTH                    2
#define RESP_B_PORT_ID_INDEX                0
#define RESP_B_TIME_NODE_LEN_INDEX          1
#define RESP_B_TYPE_INDEX                   (NODE_TYPE_INDEX + RESP_B_HEADER_LEN)

/******************************************************************
                             Server层
 Server层只是简单的在Board层的数据包前加入表示Board层数据包的长度
 字段，格式如下：
             < BoardPacketLength(2B) | BoardPacket >
 ******************************************************************/
#define CMD_S_HEADER_LEN                    2
#define RESP_S_HEADER_LEN                   2

#endif
