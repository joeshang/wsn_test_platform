/**
 * @file oint_gpio.h
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-02
 */

#ifndef __OINT_GPIO_H_
#define __OINT_GPIO_H_

/*
 * LED灯（输出）
 *  低电平--LED灯亮
 *  高电平--LED灯灭
 */
#define LED_ON                  0
#define LED_OFF                 1 

/*
 * 电流控制采样开关（输出）
 *  低电平--关闭
 *  高电平--打开
 */
#define CURR_SWITCH_OFF         0
#define CURR_SWITCH_ON          1

/*
 * FPGA复位开关（输出）
 *  低电平--FPGA复位
 *  高电平--FPGA正常工作
 */
#define FPGA_RESET_ENABLE       0
#define FPGA_RESET_DISABLE      1

/*
 * FPGA的SDRAM状态（输入）
 *  低电平--FPGA的SDRAM有数据
 *  高电平--FPGA的SDRAM为空
 */
#define FPGA_SDRAM_HAS_DATA     0
#define FPGA_SDRAM_EMPTY        1

/* 每一项对应驱动中GPIO端口描述数组oint_gpio的索引，外部不应该使用，修改需小心 */
enum _GpioIndex
{
    OINT_LED_RED_S1,
    OINT_LED_RED_S2,
    OINT_LED_RED_S3,
    OINT_LED_RED_S4,
    OINT_LED_RED_S5,
    OINT_LED_RED_S6,
    OINT_LED_RED_S7,
    OINT_LED_RED_S8,
    OINT_LED_GREEN_S1,
    OINT_LED_GREEN_S2,
    OINT_LED_GREEN_S3,
    OINT_LED_GREEN_S4,
    OINT_LED_GREEN_S5,
    OINT_LED_GREEN_S6,
    OINT_LED_GREEN_S7,
    OINT_LED_GREEN_S8,
    OINT_CURR_SWITCH,
    OINT_FPGA_RESET,
    OINT_FPGA_SDRAM_STATUS
};

/* ioctl命令 */
#define OINT_GPIO_IOC_MAGIC                 'o'
#define OINT_GPIO_IOC_LED_RED_S1            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S1, int)
#define OINT_GPIO_IOC_LED_RED_S2            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S2, int)
#define OINT_GPIO_IOC_LED_RED_S3            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S3, int)
#define OINT_GPIO_IOC_LED_RED_S4            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S4, int)
#define OINT_GPIO_IOC_LED_RED_S5            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S5, int)
#define OINT_GPIO_IOC_LED_RED_S6            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S6, int)
#define OINT_GPIO_IOC_LED_RED_S7            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S7, int)
#define OINT_GPIO_IOC_LED_RED_S8            _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_RED_S8, int)
#define OINT_GPIO_IOC_LED_GREEN_S1          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S1, int)
#define OINT_GPIO_IOC_LED_GREEN_S2          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S2, int)
#define OINT_GPIO_IOC_LED_GREEN_S3          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S3, int)
#define OINT_GPIO_IOC_LED_GREEN_S4          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S4, int)
#define OINT_GPIO_IOC_LED_GREEN_S5          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S5, int)
#define OINT_GPIO_IOC_LED_GREEN_S6          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S6, int)
#define OINT_GPIO_IOC_LED_GREEN_S7          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S7, int)
#define OINT_GPIO_IOC_LED_GREEN_S8          _IOW(OINT_GPIO_IOC_MAGIC, OINT_LED_GREEN_S8, int)
#define OINT_GPIO_IOC_CURR_SWITCH           _IOW(OINT_GPIO_IOC_MAGIC, OINT_CURR_SWITCH, int)
#define OINT_GPIO_IOC_FPGA_RESET            _IOW(OINT_GPIO_IOC_MAGIC, OINT_FPGA_RESET, int)
#define OINT_GPIO_IOC_FPGA_SDRAM_STATUS     _IOR(OINT_GPIO_IOC_MAGIC, OINT_FPGA_SDRAM_STATUS, int)

#endif
