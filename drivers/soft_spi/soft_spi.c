/*
 * Copyright (C) 2017 Hamburg University of Applied Sciences
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
 * @author      Peter Kietzmann     <peter.kietzmann@haw-hamburg.de>
 */

#include "assert.h"
#include "periph/spi.h"
#include "periph/gpio.h"
#include "soft_spi.h"
#include "soft_spi_params.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Allocate one lock per SPI device
 */
static mutex_t locks[1];

static inline bool spi_bus_is_valid(spi_t bus)
{
    unsigned int spi_num = (unsigned int) bus;

    if (sizeof(spi_config) / sizeof(spi_config[0]) < spi_num || spi_num < 0) {
        return false;
    }
    return true;
}

void spi_init(spi_t bus)
{
    DEBUG("Soft SPI init\n");
    /* This is left empty because the periph_init function will call this for
       number of hardware spi ports */
    assert(spi_bus_is_valid(bus));

    /* initialize device lock */
    mutex_init(&locks[bus]);
    spi_init_pins(bus);
}

void spi_init_pins(spi_t bus)
{
    DEBUG("Soft SPI spi_init_pins\n");

    assert(spi_bus_is_valid(bus));

    /* check that miso is not mosi is not clk*/
    assert(soft_spi_config[bus].mosi_pin != soft_spi_config[bus].miso_pin);
    assert(soft_spi_config[bus].mosi_pin != soft_spi_config[bus].clk_pin);
    assert(soft_spi_config[bus].miso_pin != soft_spi_config[bus].clk_pin);
    /* mandatory pins */
    assert((GPIO_UNDEF != soft_spi_config[bus].mosi_pin) || (GPIO_UNDEF != soft_spi_config[bus].miso_pin));
    assert(GPIO_UNDEF != soft_spi_config[bus].clk_pin);

    /* initialize clock pin */
    gpio_init(soft_spi_config[bus].clk_pin, GPIO_OUT);
    /* initialize optional pins */
    if (GPIO_UNDEF != soft_spi_config[bus].mosi_pin) {
        gpio_init(soft_spi_config[bus].mosi_pin, GPIO_OUT);
        gpio_clear(soft_spi_config[bus].mosi_pin);
    }
    if (GPIO_UNDEF != soft_spi_config[bus].miso_pin) {
        gpio_init(soft_spi_config[bus].miso_pin, GPIO_IN);
    }
}

int spi_init_cs(spi_t bus, spi_cs_t cs)
{
    DEBUG("Soft SPI init CS\n");
    if (!spi_bus_is_valid(bus)) {
        DEBUG("Soft SPI bus not valid\n");
        return SPI_NODEV;
    }

    if (cs == GPIO_UNDEF) {
        if (soft_spi_config[bus].cs_pin == GPIO_UNDEF) {
            DEBUG("Soft SPI no valid CS pin\n");
            return SPI_NOCS;
        }
        DEBUG("Soft SPI set default CS line\n");
        gpio_init(soft_spi_config[bus].cs_pin, GPIO_OUT);
        gpio_set(soft_spi_config[bus].cs_pin);
    }
    else {
        DEBUG("Soft SPI set user CS line\n");
        gpio_init(cs, GPIO_OUT);
        gpio_set(cs);
    }

    return SPI_OK;
}

int spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
{
    /* Right now we don't care about speed */
    (void)clk;

    assert(spi_bus_is_valid(bus));

    /* lock bus */
    mutex_lock(&locks[bus]);

    if (mode < SPI_MODE_0 || SPI_MODE_3 < mode) {
        return SPI_NOMODE;
    }
    soft_spi_config[bus].spi_mode = mode;
    switch (mode) {
        case SPI_MODE_0:
        case SPI_MODE_1:
            /* CPOL=0 */
            gpio_clear(soft_spi_config[bus].clk_pin);
            break;
        case SPI_MODE_2:
        case SPI_MODE_3:
            /* CPOL=1 */
            gpio_set(soft_spi_config[bus].clk_pin);
            break;
    }
    return SPI_OK;
}

void spi_release(spi_t bus)
{
    assert(spi_bus_is_valid(bus));
    mutex_unlock(&locks[bus]);
}

uint8_t _transfer_one_byte(spi_t bus, spi_cs_t cs, bool cont, uint8_t out)
{
    assert(spi_bus_is_valid(bus));

    int8_t bit = 0, i = 0;
    if (SPI_MODE_1 == soft_spi_config[bus].spi_mode ||
        SPI_MODE_3 == soft_spi_config[bus].spi_mode) {
        /* CPHA = 1*/
        gpio_toggle(soft_spi_config[bus].clk_pin);
    }

    bit = (out & (1 << 7)) >> 7;
    gpio_write(soft_spi_config[bus].mosi_pin, bit);
    for (i = 6; i >= 0; i--) {
        gpio_toggle(soft_spi_config[bus].clk_pin);
        gpio_toggle(soft_spi_config[bus].clk_pin);
        bit = (out & (1 << i)) >> i;
        gpio_write(soft_spi_config[bus].mosi_pin, bit);
    }
    gpio_toggle(soft_spi_config[bus].clk_pin);

    if (SPI_MODE_0 == soft_spi_config[bus].spi_mode ||
        SPI_MODE_2 == soft_spi_config[bus].spi_mode) {
        /* CPHASE = 1 */
        gpio_toggle(soft_spi_config[bus].clk_pin);
    }

    return out;
}

void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len)
{
    DEBUG("Soft SPI spi_transfer_bytes\n");
    uint8_t retval = 0;

    /* active the given chip select line */
    if ((cs != GPIO_UNDEF) && (cs != SPI_CS_UNDEF)) {
        gpio_clear((gpio_t)cs);
    }
    else{
        gpio_clear(soft_spi_config[bus].cs_pin);
    }

    for (size_t i = 0; i < len; i++) {
        if (NULL != out) {
            retval = _transfer_one_byte(bus, cs, true, ((uint8_t *)out)[i]);
        }
        else {
            retval = _transfer_one_byte(bus, cs, true, 0);
        }
        if (NULL != in) {
            ((uint8_t *)in)[0] = retval;
        }
    }
    if (!cont) {
        if ((cs != GPIO_UNDEF) && (cs != SPI_CS_UNDEF)) {
            gpio_set((gpio_t)cs);
        }
        else{
            gpio_set(soft_spi_config[bus].cs_pin);
        }
    }
}