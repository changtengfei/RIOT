#define ENABLE_DEBUG (1)
#include "debug.h"
#include "soft_spi.h"

int main(void)
{
    char string[] = "Hello world";

    DEBUG("%s\n", __FUNCTION__);
    spi_cs_t spi_cs = GPIO_PIN(0, 5);
    spi_acquire(0, spi_cs, SPI_MODE_0, SPI_CLK_10MHZ);
    spi_transfer_byte(0, spi_cs, false, 0xaa);
    spi_release(0);
    spi_acquire(0, spi_cs, SPI_MODE_1, SPI_CLK_10MHZ);
    spi_transfer_byte(0, spi_cs, false, 0xaa);
    spi_release(0);
    spi_acquire(0, spi_cs, SPI_MODE_2, SPI_CLK_10MHZ);
    spi_transfer_byte(0, spi_cs, false, 0xaa);
    spi_release(0);
    spi_acquire(0, spi_cs, SPI_MODE_3, SPI_CLK_10MHZ);
    spi_transfer_byte(0, spi_cs, false, 0xaa);
    spi_release(0);
    spi_acquire(0, spi_cs, SPI_MODE_0, SPI_CLK_10MHZ);
    spi_transfer_bytes(0, spi_cs, false, string, NULL, sizeof string);
    spi_release(0);
    return 0;
}
