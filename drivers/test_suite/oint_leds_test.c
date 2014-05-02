/**
 * @file oint_leds_test.c
 * @brief The test suite for OINT led driver.
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-02
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "oint_leds.h"

#define LED_DEV    "/dev/oint_leds"

static void operate_led_range(int fd, int start, int end, int operation)
{
    int i;
    for (i = start; i <= end; i++)
    {
        if (ioctl(fd, operation, i) < 0)
        {
            fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    int fd;

    if ((fd = open(LED_DEV, O_RDWR)) < 0)
    {
        fprintf(stderr, "open %s failed: %s\n", LED_DEV, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        /* all red leds on */
        operate_led_range(fd, OINT_LED_RED_S1, OINT_LED_RED_S8, LED_ON);
        sleep(1);

        /* all green leds on */
        operate_led_range(fd, OINT_LED_GREEN_S1, OINT_LED_GREEN_S8, LED_ON);
        sleep(1);

        /* all red leds off */
        operate_led_range(fd, OINT_LED_RED_S1, OINT_LED_RED_S8, LED_OFF);
        sleep(1);

        /* all green leds off */
        operate_led_range(fd, OINT_LED_GREEN_S1, OINT_LED_GREEN_S8, LED_OFF);
        sleep(1);
    }

    return 0;
}

