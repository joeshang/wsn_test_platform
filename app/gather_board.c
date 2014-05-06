/**
 * @file gather_board.c
 * @brief 封装与FPGA板的通信（包括GPIO跟SPI）
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "gather_board.h"

#define DEV_NAME_GPIO           "/dev/oint_gpio"
#define DEV_NAME_SPI_READ
#define DEV_NAME_SPI_WRITE      

#define LED_MIN_INDEX           1
#define LED_MAX_INDEX           8

static int led_ioctl_command[] = {
    OINT_GPIO_IOC_LED_RED_S1,
    OINT_GPIO_IOC_LED_RED_S2,
    OINT_GPIO_IOC_LED_RED_S3,
    OINT_GPIO_IOC_LED_RED_S4,
    OINT_GPIO_IOC_LED_RED_S5,
    OINT_GPIO_IOC_LED_RED_S6,
    OINT_GPIO_IOC_LED_RED_S7,
    OINT_GPIO_IOC_LED_RED_S8,
    OINT_GPIO_IOC_LED_GREEN_S1,
    OINT_GPIO_IOC_LED_GREEN_S2,
    OINT_GPIO_IOC_LED_GREEN_S3,
    OINT_GPIO_IOC_LED_GREEN_S4,
    OINT_GPIO_IOC_LED_GREEN_S5,
    OINT_GPIO_IOC_LED_GREEN_S6,
    OINT_GPIO_IOC_LED_GREEN_S7,
    OINT_GPIO_IOC_LED_GREEN_S8,
};

struct _GatherBoard
{
    int gpio_fd;
    int spi_read_fd;
    int spi_write_fd;
};

GatherBoard *gather_board_create()
{
    debug("[GatherBoard]: create\n");

    GatherBoard *thiz = (GatherBoard *)malloc(sizeof(GatherBoard));

    if (thiz != NULL)
    {
        /* open gpio driver */
        if ((thiz->gpio_fd = open(DEV_NAME_GPIO, O_RDWR)) == -1)
        {
            fprintf(stderr, "open device %s failed\n", DEV_NAME_GPIO);
            exit(EXIT_FAILURE);
        }

        /* open spi read driver */
        /* open spi write driver */

    }

    return thiz;
}

void gather_board_destroy(GatherBoard *thiz)
{
    debug("[GatherBoard]: destroy\n");

    if (thiz != NULL)
    {
        close(thiz->gpio_fd);
        close(thiz->spi_read_fd);
        close(thiz->spi_write_fd); 

        free(thiz);
    }
}

static Ret gather_board_led_control(GatherBoard *thiz, int led_index, int value, int red)
{
    debug("[GatherBoard]: %s led %d -> %d\n", (red == TRUE ? "red" : "green"), led_index, value);

    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail((value != LED_ON && value != LED_ON), 
            RET_INVALID_PARAMS);
    return_val_if_fail((led_index < LED_MIN_INDEX && led_index > LED_MAX_INDEX),
            RET_INVALID_PARAMS);

    int base;

    if (red == TRUE)
    {
        base = 0;
    }
    else
    {    
        base = LED_MAX_INDEX;
    }

    if (ioctl(thiz->gpio_fd, led_ioctl_command[base + led_index - 1], value) == -1)
    {
        fprintf(stderr, "led control ioctl failed: %s\n", strerror(errno));
        return RET_FAIL;
    }

    return RET_OK;
}

Ret  gather_board_led_red(GatherBoard *thiz, int led_index, int value)
{
    return gather_board_led_control(thiz, led_index, value, TRUE);
}

Ret  gather_board_led_green(GatherBoard *thiz, int led_index, int value)
{
    return gather_board_led_control(thiz, led_index, value, FALSE);
}

Ret  gather_board_current_switch(GatherBoard *thiz, int value)
{
    debug("[GatherBoard]: current sample switch -> %d\n", value);

    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail((value != CURR_SWITCH_OFF && value != CURR_SWITCH_ON), 
            RET_INVALID_PARAMS);

    if (ioctl(thiz->gpio_fd, OINT_GPIO_IOC_CURR_SWITCH, value) == -1)
    {
        fprintf(stderr, "current switch ioctl failed: %s\n", strerror(errno));
        return RET_FAIL;
    }

    return RET_OK;
}

Ret  gather_board_reset_fpga(GatherBoard *thiz)
{
    debug("[GatherBoard]: reset fpga\n");

    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    gather_board_current_switch(thiz, CURR_SWITCH_OFF);

    if (ioctl(thiz->gpio_fd, OINT_GPIO_IOC_FPGA_RESET, FPGA_RESET_ENABLE) == -1)
    {
        fprintf(stderr, "fpga reset ioctl failed: %s\n", strerror(errno));
        return RET_FAIL;
    }

    usleep(2);

    if (ioctl(thiz->gpio_fd, OINT_GPIO_IOC_FPGA_RESET, FPGA_RESET_DISABLE) == -1)
    {
        fprintf(stderr, "fpga reset ioctl failed: %s\n", strerror(errno));
        return RET_FAIL;
    }

    printf("[GatherBoard]: reset fpga ok\n");

    return RET_OK;
}
