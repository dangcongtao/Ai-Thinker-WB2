#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include <hosal_spi.h>
#include <bl_gpio.h>
#include <blog.h>

static hosal_spi_dev_t spi1 = {
    .cb = NULL,
    .config = {
        .dma_enable = 0, /* enable not run */
        .freq = 2000000, /** 4MHz */
        .mode = HOSAL_SPI_MODE_MASTER,
        .pin_clk = 0xFF, /* not use */
        .pin_miso = 0xFF, /* not use */
        .pin_mosi = 12,
        .polar_phase = 0,
    },
    .p_arg = NULL,
    .port = 0,
};


/** data set for IC led RGB UCS1903, in low speed mode */
/* color 1, expect red */
uint8_t test_col1[22] = {0x0, 0x0,
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        };
/* color 2, expect green */
uint8_t test_col2[22] = {0x0, 0x0,
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        };
/* color 3, expect blue */
uint8_t test_col3[22] = {0x0, 0x0,
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        };

/* white */
uint8_t test_white[22] = {0x0, 0x0,
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        0xF7, 0xBD, 0xEF, 0x7B, 0xDE, /* 255*/
                        };

/* turn off */
uint8_t test_off[22] = {0x0, 0x0,
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        0x84, 0x21, 0x8, 0x42, 0x10, /* 0 */
                        };

int main(void)
{
    /* disable buzzer */
    bl_gpio_enable_output(1, 1, 0);
    bl_gpio_output_set(1, 0);
    /* init */
    hosal_spi_init(&spi1);

    /** change color automaticly every 2secs */
    for (;;)
    {
        blog_warn("off 1 ");
        hosal_spi_send(&spi1, test_col1, sizeof(test_col1), 500);
        vTaskDelay(portTICK_RATE_MS * 2000);

        blog_warn("off 2 ");
        hosal_spi_send(&spi1, test_col2, sizeof(test_col2), 500);
        vTaskDelay(portTICK_RATE_MS * 2000);

        blog_warn("off 3 ");
        hosal_spi_send(&spi1, test_col3, sizeof(test_col3), 500);
        vTaskDelay(portTICK_RATE_MS * 2000);

        blog_error("off");
        hosal_spi_send(&spi1, test_off, sizeof(test_off), 500);
        vTaskDelay(portTICK_RATE_MS * 2000);
    }
    return 0;
}