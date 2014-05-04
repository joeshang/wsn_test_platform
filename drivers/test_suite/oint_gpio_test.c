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

#include "oint_gpio.h"

#define GPIO_DEV    "/dev/oint_gpio"

static char menu[] = \
"=== OINT GPIO Test Suite Menu ===\n\
<1> Led Blink \n\
<2> Current Sample Switch \n\
<3> FPGA Reset \n\
<4> FPGA SDRAM Status \n\
<0> Exit \n\
Please input selection: ";

static void operate_led_range(int fd, int start, int end, int operation)
{
    int i;
    for (i = start; i <= end; i++)
    {
        if (ioctl(fd, i, operation) < 0)
        {
            fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

static void led_blink(int fd, int count)
{
    printf("led blinking...\n");

    int i = 0;

    for (i = 0; i < count; i++)
    {
        /* all red leds on */
        operate_led_range(fd, OINT_GPIO_IOC_LED_RED_S1, OINT_GPIO_IOC_LED_RED_S8, LED_ON);
        sleep(1);

        /* all green leds on */
        operate_led_range(fd, OINT_GPIO_IOC_LED_GREEN_S1, OINT_GPIO_IOC_LED_GREEN_S8, LED_ON);
        sleep(1);

        /* all red leds off */
        operate_led_range(fd, OINT_GPIO_IOC_LED_RED_S1, OINT_GPIO_IOC_LED_RED_S8, LED_OFF);
        sleep(1);

        /* all green leds off */
        operate_led_range(fd, OINT_GPIO_IOC_LED_GREEN_S1, OINT_GPIO_IOC_LED_GREEN_S8, LED_OFF);
        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    int fd;
    int selection = -1;

    if ((fd = open(GPIO_DEV, O_RDWR)) < 0)
    {
        fprintf(stderr, "open %s failed: %s\n", GPIO_DEV, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        printf("%s", menu);
        scanf("%d", &selection); 
        printf("\n");
        
        switch (selection)
        {
            /* Exit */
            case 0:
                return 0;
            /* Led Blink */
            case 1:
            {
                int count = -1;
                printf("Please input blink times(should <= 10): ");
                scanf("%d", &count);
                printf("\n");

                if (count > 10)
                {
                    count = 10;
                }
                led_blink(fd, count);

                break;
            }
            /* Current Sample Switch */
            case 2:
            {
                int oper = -1;
                printf("Please input [switch_off(0)/switch_on(1)]: ");
                scanf("%d", &oper);
                printf("\n");

                if (oper == 0 || oper == 1)
                {
                    if (ioctl(fd, OINT_GPIO_IOC_CURR_SWITCH, oper) < 0)
                    {
                        fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    printf("Invalid input selection\n");
                }
                break;
            }
            /* FPGA Reset */
            case 3:
            {
                int oper = -1;
                printf("Please input [reset(0)/no_reset(1)]: ");
                scanf("%d", &oper);
                printf("\n");

                if (oper == 0 || oper == 1)
                {
                    if (ioctl(fd, OINT_GPIO_IOC_FPGA_RESET, oper) < 0)
                    {
                        fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    printf("Invalid input selection\n");
                }
                break;
            }
            /* FPAG SDRAM Status */
            case 4:
            {
                int status;
                if (ioctl(fd, OINT_GPIO_IOC_FPGA_SDRAM_STATUS, &status) < 0)
                {
                    fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                if (status == FPGA_SDRAM_HAS_DATA)
                {
                    printf("FPGA SDRAM status: SDRAM with data\n");
                }
                else if (status == FPGA_SDRAM_EMPTY)
                {
                    printf("FPGA SDRAM status: SDRAM empty\n");
                }

                break;
            }
            default:
                break;
        }
    }

    return 0;
}

