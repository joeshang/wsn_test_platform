/**
 * @file oint_protocol.h
 * @brief 定义通信协议中各个字段的索引，以及控制命令
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#ifndef __OINT_PROTOCOL_H__
#define __OINT_PROTOCOL_H__

/* 定义ARM与FPGA通信协议（其实就是FPGA与节点通信协议）, 前缀为CMD_B，B代表硬件板 */
#define CMD_B_TYPE_INDEX                11

/* 上报数据的类型 */
#define CMD_B_TYPE_UPLOAD_RECV          0x11    /* 抄送无线接收分组信息 */
#define CMD_B_TYPE_UPLOAD_SEND          0x12    /* 抄送无线发送分组信息 */
#define CMD_B_TYPE_UPLOAD_NODE_NUM      0x13    /* 上报节点号 */
#define CMD_B_TYPE_UPLOAD_CHANNEL       0x14    /* 上报信道 */
#define CMD_B_TYPE_UPLOAD_RF_POWER      0x15    /* 上报射频功率 */
#define CMD_B_TYPE_UPLOAD_VENDOR        0x16    /* 上报生产厂商 */
#define CMD_B_TYPE_UPLOAD_SENSOR_CNT    0x17    /* 上报传感器数量 */
#define CMD_B_TYPE_UPLOAD_NODE_ID       0x18    /* 上报节点标识 */
#define CMD_B_TYPE_UPLOAD_CURRENT       0x19    /* 上报电流值 */
#define CMD_B_TYPE_UPLOAD_RANGE         0x2F    /* 上报支持的测试范围 */
/* 查询和设置命令的类型 */
#define CMD_B_TYPE_QUERY_RECV           0x31    /* 设置抄送无线接收分组信息 */
#define CMD_B_TYPE_QUERY_SEND           0x32    /* 设置抄送无线发送分组信息 */
#define CMD_B_TYPE_QUERY_NODE_NUM       0x33    /* 查询节点号 */
#define CMD_B_TYPE_QUERY_CHANNEL        0x34    /* 查询信道 */
#define CMD_B_TYPE_QUERY_RF_POWER       0x35    /* 查询射频功率 */
#define CMD_B_TYPE_QUERY_VENDOR         0x36    /* 查询生产厂商 */
#define CMD_B_TYPE_QUERY_SENSOR_CNT     0x37    /* 查询传感器数量 */
#define CMD_B_TYPE_QUERY_NODE_ID        0x38    /* 查询节点标识 */
#define CMD_B_TYPE_QUERY_CURRENT        0x39    /* 查询节点电流值 */
#define CMD_B_TYPE_QUERY_RANGE          0x4F    /* 查询支持的测试范围 */
/* 控制命令的类型 */
#define CMD_B_TYPE_CNTL_MODIFY_CHANNEL  0x51    /* 修改信道 */
#define CMD_B_TYPE_CNTL_MODIFY_POWER    0x52    /* 修改功率 */
#define CMD_B_TYPE_CNTL_REMOTE_PROG     0x53    /* 远程编程 */
#define CMD_B_TYPE_CNTL_POWER_OFF       0x54    /* 断电 */

/* 定义上位机与ARM板通信协议，前缀为CMD_U，U代表上位机 */
#define CMD_U_LENGTH_WIDTH              2
#define CMD_U_PORT_ID_WIDTH             1

#define RESP_U_LENGTH_WIDTH             2
#define RESP_U_PORT_ID_WIDTH            1
#define RESP_U_TIMESTAMP_WIDTH          4

#endif
