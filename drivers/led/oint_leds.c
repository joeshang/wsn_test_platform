/**
 * @file oint_leds.c
 * @brief Led driver for OINT
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

#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include "oint_leds.h"

#define LED_NUM         ARRAY_SIZE(led_gpios) 
#define DEVICE_NAME     "oint_leds"

static int led_gpios[] = {
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
};

static long oint_leds_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    if (arg > LED_NUM)
    {
        printk("invalid led index: %lu\n", arg);
        return -EINVAL;
    }

    switch (cmd)
    {
        case LED_ON:
        case LED_OFF:
            gpio_set_value(led_gpios[arg], cmd);
            break;
        default:
            printk("invalid led command: %u\n", cmd);
            return -EINVAL;
    }

    return 0;
}

static struct file_operations oint_leds_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = oint_leds_ioctl
};

static struct miscdevice oint_leds_dev = {
    .minor          = MISC_DYNAMIC_MINOR,
    .name           = DEVICE_NAME,
    .fops           = &oint_leds_fops
};

static int __init oint_leds_init(void)
{
    int ret;
    int i;

    /* init gpio */
    for (i = 0; i < LED_NUM; i++)
    {
        ret = gpio_request(led_gpios[i], "OINT LED");
        if (ret)
        {
            printk("%s: request GPIO %d for LED failed, ret = %d\n", DEVICE_NAME, led_gpios[i], ret);
            return ret;
        }
        s3c_gpio_cfgpin(led_gpios[i], S3C_GPIO_OUTPUT);
        gpio_set_value(led_gpios[i], LED_OFF);
    }

    /* register device */
    ret = misc_register(&oint_leds_dev);

    printk("init OINT led driver");

    return ret;
}

static void __exit oint_leds_cleanup(void)
{
    int i; 

    for (i = 0; i < LED_NUM; i++)
    {
        gpio_free(led_gpios[i]);
    }

    misc_deregister(&oint_leds_dev);

    printk("cleanup OINT led driver");
}

module_init(oint_leds_init);
module_exit(oint_leds_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Joe Shang");
MODULE_DESCRIPTION("The led driver for OINT");
