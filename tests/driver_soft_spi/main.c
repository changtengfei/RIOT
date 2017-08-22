/*
 * Copyright (C) 2017 Hamburg University of Applied Sciences
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Application for testing the software SPI driver implementations
 *
 *
 * @author      Markus Blechschmidt <Markus.Blechschmidt@haw-hamburg.de>
 * @author      Peter Kietzmann     <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "periph/spi.h"
#include "soft_spi.h"

#ifdef SPI_NUMOF
#pragma message("WARNING! All hardware SPI devices will be disabled.")
#undef SPI_NUMOF
#endif
//#define SPI_NUMOF 0

int main(void)
{
    puts("Minimal test application for the software SPI driver");

    char string[] = "Soft SPI Test String";

    /* Initialize software SPI device. This function is usually
       called during board initialization*/
    spi_init(SPI_DEV(0));

    /* Initialize CS pin */
    int tmp = spi_init_cs(SPI_DEV(0), GPIO_UNDEF);
    if (tmp != SPI_OK) {
        printf("error: unable to initialize the given chip select line %i\n", tmp);
        return 1;
    }

    puts("Send 0xab");
    spi_acquire(SPI_DEV(0), GPIO_UNDEF, SPI_MODE_0, 0);
    spi_transfer_byte(SPI_DEV(0), GPIO_UNDEF, false, 0xaa);
    spi_release(SPI_DEV(0));

    printf("Send %s\n",string);
    spi_acquire(SPI_DEV(0), GPIO_UNDEF, SPI_MODE_0, 0);
    spi_transfer_bytes(SPI_DEV(0), GPIO_UNDEF, false, string, NULL, sizeof string);
    spi_release(SPI_DEV(0));

    puts("Soft SPI Test End");
    return 0;
}
