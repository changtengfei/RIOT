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
 * @brief       Software SPI configuration
 *
 * @author      Markus Blechschmidt <Markus.Blechschmidt@haw-hamburg.de>
 */

#ifndef SOFT_SPI_CONF_H
#define SOFT_SPI_CONF_H

#include "soft_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SOFT_SPI_PARAM_CLK
#define SOFT_SPI_PARAM_CLK         (GPIO_PIN(0, 6))
#endif
#ifndef SOFT_SPI_PARAM_MOSI
#define SOFT_SPI_PARAM_MOSI        (GPIO_PIN(0, 7))
#endif
#ifndef SOFT_SPI_PARAM_MISO
#define SOFT_SPI_PARAM_MISO        (GPIO_UNDEF)
#endif


/**
 * @brief   Sotware SPI port descriptor array
 */
soft_spi_t soft_spi_config[] = {
    {
        .mosi_pin = SOFT_SPI_PARAM_MOSI,
        .miso_pin = SOFT_SPI_PARAM_MISO,
        .clk_pin = SOFT_SPI_PARAM_CLK,
        .initialized = false,
    },
};

#ifdef __cplusplus
}
#endif

#endif /* SOFT_SPI_CONF_H */
/** @} */
