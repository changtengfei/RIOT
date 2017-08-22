/*
 * Copyright (C) 2017 Hamburg University of Applied Sciences
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    driver_soft_spi Soft SPI
 * @ingroup     drivers
 * @brief       Software implemented Serial Peripheral Interface Bus
 * @{
 *
 * @file
 * @brief       Software SPI port descriptor definition
 *
 * @author      Markus Blechschmidt <Markus.Blechschmidt@haw-hamburg.de>
 */

#ifndef SOFT_SPI_H
#define SOFT_SPI_H

#include "periph/gpio.h"
#include "periph/spi.h"
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Use the shared SPI functions
 * @{
 */
#define PERIPH_SPI_NEEDS_TRANSFER_BYTE
#define PERIPH_SPI_NEEDS_TRANSFER_REG
#define PERIPH_SPI_NEEDS_TRANSFER_REGS

/**
 * @brief Software SPI port descriptor
 */
typedef struct {
    gpio_t miso_pin;              /**<MOSI pin */
    gpio_t mosi_pin;              /**<MOSI pin */
    gpio_t clk_pin;               /**<CLK pin */
    gpio_t cs_pin ;               /**<CS  pin */
    spi_mode_t spi_mode;          /**<data and clock polarity */
} soft_spi_t;


#ifdef __cplusplus
}
#endif

#endif /* SOFT_SPI_H */
/** @} */
