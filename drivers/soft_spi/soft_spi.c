/*
 * Copyright (C) 2017 Markus Blechschmidt <Markus.Blechschmidt@haw-hamburg.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_soft_spi
 * @{
 *
 * @file
 * @brief       Software SPI implementation
 *
 * @author      Markus Blechschmidt <Markus.Blechschmidt@haw-hamburg.de>
 */

#include "assert.h"
#include "soft_spi.h"
#include "periph/spi.h"
#include "periph/gpio.h"
#include "soft_spi_params.h"
#define ENABLE_DEBUG (0)
#include "debug.h"

static bool spi_bus_is_valid(spi_t bus)
{
    unsigned int spi_num = (unsigned int) bus;

    if (sizeof(spi_config) / sizeof(spi_config[0]) < spi_num || spi_num < 0) {
        return false;
    }
    return true;
}

void spi_init(spi_t bus)
{
    /* This is left empty because the periph_init function will call this for
       number of hardware spi ports */
}

void spi_init_pins(spi_t bus)
{
    soft_spi_t *curr_soft_spi;

    assert(spi_bus_is_valid(bus));
    curr_soft_spi = &soft_spi_config[(unsigned int) bus];
    /* check that miso is not mosi is not clk*/
    assert(curr_soft_spi->mosi_pin != curr_soft_spi->miso_pin);
    assert(curr_soft_spi->mosi_pin != curr_soft_spi->clk_pin);
    assert(curr_soft_spi->miso_pin != curr_soft_spi->clk_pin);
    /* mandatory pins */
    assert((GPIO_UNDEF != curr_soft_spi->mosi_pin) || (GPIO_UNDEF != curr_soft_spi->miso_pin));
    assert(GPIO_UNDEF != curr_soft_spi->clk_pin);
    /* check and setup mode related things */
    assert(curr_soft_spi->spi_mode >= SPI_MODE_0);
    assert(curr_soft_spi->spi_mode <= SPI_MODE_3);
    /* initialize clock pin */
    gpio_init(curr_soft_spi->clk_pin, GPIO_OUT);
    /* initialize optional pins */
    if (GPIO_UNDEF != curr_soft_spi->mosi_pin) {
        gpio_init(curr_soft_spi->mosi_pin, GPIO_OUT);
        gpio_clear(curr_soft_spi->mosi_pin);
    }
    if (GPIO_UNDEF != curr_soft_spi->miso_pin) {
        gpio_init(curr_soft_spi->miso_pin, GPIO_IN);
    }
}

int spi_init_cs(spi_t bus, spi_cs_t cs)
{
    if (!spi_bus_is_valid(bus)) {
        return SPI_NODEV;
    }
    if (GPIO_UNDEF == cs) {
        return SPI_NOCS;
    }
    /* unselect chip */
    gpio_init(cs, GPIO_OUT);
    gpio_set(cs);
    return SPI_OK;
}

int spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
{
    soft_spi_t *curr_soft_spi;

    assert(spi_bus_is_valid(bus));
    curr_soft_spi = &soft_spi_config[(unsigned int) bus];

    if (!curr_soft_spi->initialized) {
        mutex_init(&(curr_soft_spi->lock));
    }
    mutex_lock(&(curr_soft_spi->lock));

    if (mode < SPI_MODE_0 || SPI_MODE_3 < mode) {
        return SPI_NOMODE;
    }
    curr_soft_spi->spi_mode = mode;
    switch (mode) {
        case SPI_MODE_0:
        case SPI_MODE_1:
            /* CPOL=0 */
            gpio_clear(curr_soft_spi->clk_pin);
            break;
        case SPI_MODE_2:
        case SPI_MODE_3:
            /* CPOL=1 */
            gpio_set(curr_soft_spi->clk_pin);
            break;
    }
    /* initialize pins */
    if (!curr_soft_spi->initialized) {
        spi_init_pins(bus);
    }
    spi_init_cs(bus, cs);
    curr_soft_spi->initialized = true;
    return SPI_OK;
}

void spi_release(spi_t bus)
{
    soft_spi_t *curr_soft_spi;

    assert(spi_bus_is_valid(bus));
    curr_soft_spi = &soft_spi_config[(unsigned int) bus];
    mutex_unlock(&((*curr_soft_spi).lock));
}

uint8_t spi_transfer_byte(spi_t bus, spi_cs_t cs, bool cont, uint8_t out)
{
    soft_spi_t *curr_soft_spi;

    assert(spi_bus_is_valid(bus));
    curr_soft_spi = &soft_spi_config[(unsigned int) bus];

    int8_t bit = 0, i = 0;
    gpio_clear(cs);
    if (SPI_MODE_1 == curr_soft_spi->spi_mode ||
        SPI_MODE_3 == curr_soft_spi->spi_mode) {
        /* CPHA = 1*/
        gpio_toggle(curr_soft_spi->clk_pin);
    }

    bit = (out & (1 << 7)) >> 7;
    gpio_write(curr_soft_spi->mosi_pin, bit);
    for (i = 6; i >= 0; i--) {
        gpio_toggle(curr_soft_spi->clk_pin);
        gpio_toggle(curr_soft_spi->clk_pin);
        bit = (out & (1 << i)) >> i;
        gpio_write(curr_soft_spi->mosi_pin, bit);
    }
    gpio_toggle(curr_soft_spi->clk_pin);

    if (SPI_MODE_0 == curr_soft_spi->spi_mode ||
        SPI_MODE_2 == curr_soft_spi->spi_mode) {
        /* CPHASE = 1 */
        gpio_toggle(curr_soft_spi->clk_pin);
    }

    if (!cont) {
        gpio_set(cs);
    }

    gpio_clear(curr_soft_spi->mosi_pin);
    return out;
}

void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len)
{
    uint8_t retval = 0;

    for (size_t i = 0; i < len; i++) {
        if (NULL != out) {
            retval = spi_transfer_byte(bus, cs, true, ((uint8_t *)out)[i]);
        }
        else {
            retval = spi_transfer_byte(bus, cs, true, 0);
        }
        if (NULL != in) {
            ((uint8_t *)in)[0] = retval;
        }
    }
    if (!cont) {
        gpio_set(cs);
    }
}

uint8_t spi_transfer_reg(spi_t bus, spi_cs_t cs, uint8_t reg, uint8_t out)
{
    spi_transfer_byte(bus, cs, true, reg);
    return spi_transfer_byte(bus, cs, false, out);
}

void spi_transfer_regs(spi_t bus, spi_cs_t cs, uint8_t reg,
                       const void *out, void *in, size_t len)
{
    spi_transfer_byte(bus, cs, true, reg);
    spi_transfer_bytes(bus, cs, false, out, in, len);
}
