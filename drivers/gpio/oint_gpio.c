/**
 * @file oint_gpio.c
 * @brief OINT中GPIO的驱动
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-30
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include "oint_gpio.h"

#define GPIO_NUM        ARRAY_SIZE(oint_gpios) 
#define DEVICE_NAME     "oint_gpio"

static int oint_gpios[] = {
    // red leds
    S3C64XX_GPE(1), // red s1
    S3C64XX_GPE(3), // red s2
    S3C64XX_GPM(0), // red s3
    S3C64XX_GPM(2), // red s4
    S3C64XX_GPM(4), // red s5
    S3C64XX_GPQ(1), // red s6
    S3C64XX_GPQ(3), // red s7
    S3C64XX_GPQ(5), // red s8
    // green leds
    S3C64XX_GPE(2), // green s1
    S3C64XX_GPE(4), // green s2
    S3C64XX_GPM(1), // green s3
    S3C64XX_GPM(3), // green s4
    S3C64XX_GPM(5), // green s5
    S3C64XX_GPQ(2), // green s6
    S3C64XX_GPQ(4), // green s7
    S3C64XX_GPQ(6), // green s8
    // current sample switch
    S3C64XX_GPN(9), // alias EINT9
    // fpga reset
    S3C64XX_GPN(11),// alias EINT11
    // fpga SDRAM status
    S3C64XX_GPL(8), // alias EINT16
};

static long oint_gpio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    int gpio_index = 0;
    int err = 0;
    int input;

    if (_IOC_TYPE(cmd) != OINT_GPIO_IOC_MAGIC)
    {
        return -EINVAL;
    }
    if (_IOC_NR(cmd) > GPIO_NUM)
    {
        return -EINVAL;
    }

    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    }
    if (err)
    {
        return -EFAULT;
    }

    gpio_index = _IOC_NR(cmd);
    switch (cmd)
    {
        case OINT_GPIO_IOC_LED_RED_S1:
        case OINT_GPIO_IOC_LED_RED_S2:
        case OINT_GPIO_IOC_LED_RED_S3:
        case OINT_GPIO_IOC_LED_RED_S4:
        case OINT_GPIO_IOC_LED_RED_S5:
        case OINT_GPIO_IOC_LED_RED_S6:
        case OINT_GPIO_IOC_LED_RED_S7:
        case OINT_GPIO_IOC_LED_RED_S8:
        case OINT_GPIO_IOC_LED_GREEN_S1:
        case OINT_GPIO_IOC_LED_GREEN_S2:
        case OINT_GPIO_IOC_LED_GREEN_S3:
        case OINT_GPIO_IOC_LED_GREEN_S4:
        case OINT_GPIO_IOC_LED_GREEN_S5:
        case OINT_GPIO_IOC_LED_GREEN_S6:
        case OINT_GPIO_IOC_LED_GREEN_S7:
        case OINT_GPIO_IOC_LED_GREEN_S8:
        case OINT_GPIO_IOC_CURR_SWITCH:
        case OINT_GPIO_IOC_FPGA_RESET:
            gpio_set_value(oint_gpios[gpio_index], arg);
            break;
        case OINT_GPIO_IOC_FPGA_SDRAM_STATUS:
            input = gpio_get_value(oint_gpios[gpio_index]);
            put_user(input, (int *)arg);
            break;
        default:
           return -EINVAL; 
    }

    return ret;
}

static struct file_operations oint_gpio_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = oint_gpio_ioctl
};

static struct miscdevice oint_gpio_dev = {
    .minor          = MISC_DYNAMIC_MINOR,
    .name           = DEVICE_NAME,
    .fops           = &oint_gpio_fops
};

static int __init oint_gpio_init(void)
{
    int ret;
    int i;

    /* init gpio */
    for (i = 0; i < GPIO_NUM; i++)
    {
        ret = gpio_request(oint_gpios[i], "OINT GPIO");
        if (ret)
        {
            printk("%s: request GPIO %d failed, ret = %d\n", DEVICE_NAME, oint_gpios[i], ret);
            return ret;
        }

        switch (i)
        {
            case OINT_LED_RED_S1:
            case OINT_LED_RED_S2:
            case OINT_LED_RED_S3:
            case OINT_LED_RED_S4:
            case OINT_LED_RED_S5:
            case OINT_LED_RED_S6:
            case OINT_LED_RED_S7:
            case OINT_LED_RED_S8:
            case OINT_LED_GREEN_S1:
            case OINT_LED_GREEN_S2:
            case OINT_LED_GREEN_S3:
            case OINT_LED_GREEN_S4:
            case OINT_LED_GREEN_S5:
            case OINT_LED_GREEN_S6:
            case OINT_LED_GREEN_S7:
            case OINT_LED_GREEN_S8:
                s3c_gpio_setpull(oint_gpios[i], S3C_GPIO_PULL_UP);
                gpio_direction_output(oint_gpios[i], LED_OFF);
                break;
            case OINT_CURR_SWITCH:
                s3c_gpio_setpull(oint_gpios[i], S3C_GPIO_PULL_UP);
                gpio_direction_output(oint_gpios[i], CURR_SWITCH_OFF);
                break;
            case OINT_FPGA_RESET:
                s3c_gpio_setpull(oint_gpios[i], S3C_GPIO_PULL_UP);
                gpio_direction_output(oint_gpios[i], FPGA_RESET_DISABLE);
                break;
            case OINT_FPGA_SDRAM_STATUS:
                s3c_gpio_setpull(oint_gpios[i], S3C_GPIO_PULL_NONE);
                gpio_direction_input(oint_gpios[i]);
                break;
            default:
                break;
        }
    }

    /* register device */
    ret = misc_register(&oint_gpio_dev);

    printk("init OINT gpio driver");

    return ret;
}

static void __exit oint_gpio_cleanup(void)
{
    int i; 

    for (i = 0; i < GPIO_NUM; i++)
    {
        gpio_free(oint_gpios[i]);
    }

    misc_deregister(&oint_gpio_dev);

    printk("cleanup OINT gpio driver");
}

module_init(oint_gpio_init);
module_exit(oint_gpio_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Joe Shang");
MODULE_DESCRIPTION("The gpio driver for OINT");
