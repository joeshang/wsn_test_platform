/**
 * @file oint_leds.c
 * @brief Led driver for OINT
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-30
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/ioctl.h>

satic int __init oint_leds_init(void)
{
    printk(KERN_ALERT "init OINT leds");

    return 0;
}

static void __exit oint_leds_cleanup(void)
{
    printk(KERN_ALERT "cleanup OINT leds");
}

module_init(oint_leds_init);
module_exit(oint_leds_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Joe Shang");
MODULE_DESCRIPTION("The led driver for OINT");
