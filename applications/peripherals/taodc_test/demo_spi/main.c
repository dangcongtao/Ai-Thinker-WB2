#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include <hosal_spi.h>
#include <bl_gpio.h>
#include <blog.h>

static hosal_spi_dev_t spi1 = {
        .cb = NULL,
        .config = {
            .dma_enable = 0,
            .freq = 2000000, /** 2MHz */
            .mode = HOSAL_SPI_MODE_MASTER,
            .pin_clk = 0xFF,
            .pin_miso = 0xFF,
            .pin_mosi = 12,
            .polar_phase = 0,
        },
        .p_arg = NULL,
        .port = 0,
    };


/* remove 1st 8 bit as start byte */
uint8_t color[6] = {0, 170, 170, 170, 170, 170};
int main(void)
{
    /* disable buzzer */
    bl_gpio_enable_output(1, 1, 0);
    bl_gpio_output_set(1, 0);

    /* init */
    hosal_spi_init(&spi1);

    for (;;) {

        hosal_spi_send(&spi1, color, 6, 500);
        vTaskDelay(portTICK_RATE_MS * 1000);
    }

    return 0;
}
