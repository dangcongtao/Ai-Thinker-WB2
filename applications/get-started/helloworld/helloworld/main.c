#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include <hosal_spi.h>
#include <bl_gpio.h>
#include <blog.h>
#include <aos/kernel.h>
#include "bl_sys.h"

#define PIN_CLK 3  /* same with demo */
#define PIN_MOSI 5 /* demo use IO12 */
#define PIN_CS 2

static hosal_spi_dev_t spi0 = {
    .cb = NULL,
    .config = {
        .dma_enable = 0,
        .freq = 1000000, /* DEMO use 10Mhz, current use 1Mhz */
        .mode = HOSAL_SPI_MODE_MASTER,
        .pin_clk = PIN_CLK,
        .pin_mosi = PIN_MOSI,
        .polar_phase = 0,
    },
    .p_arg = NULL,
    .port = 0,
};

static void demo_init_spi(void)
{
    hosal_spi_init(&spi0);

    /* for chip select */
    bl_gpio_enable_output(PIN_CS, 1, 0);
    bl_gpio_output_set(PIN_CS, 1);
}

static void demo_spi_send(void)
{
    uint8_t tx_data = 0xAA;
    printf("SPI send data: 0x%02X \r\n", tx_data);

    bl_gpio_output_set(PIN_CS, 0);
    hosal_spi_send(&spi0, &tx_data, 1, HOSAL_WAIT_FOREVER);
    bl_gpio_output_set(PIN_CS, 1);
}

uint32_t s_tick = 0;
int main(void)
{
    demo_init_spi();

    while (1)
    {
        if (aos_now_ms() - s_tick > 2000)
        {
            s_tick = aos_now_ms();
            demo_spi_send();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return 0;
}
