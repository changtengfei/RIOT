/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2016 Hamburg University of Applied Sciences
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    boards_stm32f4discovery STM32F4Discovery
 * @ingroup     boards
 * @brief       Board specific files for the STM32F4Discovery board
 * @{
 *
 * @file
 * @brief       Board specific definitions for the STM32F4Discovery evaluation board
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "cpu.h"
#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name xtimer configuration
 * @{
 */
#define XTIMER_DEV          TIMER_0
#define XTIMER_CHAN         (0)
#define XTIMER_OVERHEAD     (6)
#define XTIMER_BACKOFF      (10)
/** @} */

/**
 * @name AT86RF233 configuration for externally connected device
 *
 * {spi bus, spi speed, cs pin, int pin, reset pin, sleep pin}
 */
#define AT86RF2XX_PARAMS_BOARD      {.spi = SPI_0, \
                                     .spi_speed = SPI_SPEED_5MHZ, \
                                     .cs_pin = GPIO_PIN(PORT_A, 0), \
                                     .int_pin = GPIO_PIN(PORT_A, 1), \
                                     .sleep_pin = GPIO_PIN(PORT_D, 8), \
                                     .reset_pin = GPIO_PIN(PORT_D, 9)}

/**
 * @name LED pin definitions
 * @{
 */
/** @} */

/**
 * @name Macros for controlling the on-board LEDs.
 * @{
 */
#define LED0_PIN            GPIO_PIN(PORT_D, 13)
#define LED1_PIN            GPIO_PIN(PORT_D, 12)
#define LED2_PIN            GPIO_PIN(PORT_D, 14)
#define LED3_PIN            GPIO_PIN(PORT_D, 15)

#define LED_PORT            GPIOD
#define LED0_MASK           (1 << 13)
#define LED1_MASK           (1 << 12)
#define LED2_MASK           (1 << 14)
#define LED3_MASK           (1 << 15)

#define LED0_ON             (LED_PORT->BSRRL = LED0_MASK)
#define LED0_OFF            (LED_PORT->BSRRH = LED0_MASK)
#define LED0_TOGGLE         (LED_PORT->ODR  ^= LED0_MASK)

#define LED1_ON             (LED_PORT->BSRRL = LED1_MASK)
#define LED1_OFF            (LED_PORT->BSRRH = LED1_MASK)
#define LED1_TOGGLE         (LED_PORT->ODR  ^= LED1_MASK)

#define LED2_ON             (LED_PORT->BSRRL = LED2_MASK)
#define LED2_OFF            (LED_PORT->BSRRH = LED2_MASK)
#define LED2_TOGGLE         (LED_PORT->ODR  ^= LED2_MASK)

#define LED3_ON             (LED_PORT->BSRRL = LED3_MASK)
#define LED3_OFF            (LED_PORT->BSRRH = LED3_MASK)
#define LED3_TOGGLE         (LED_PORT->ODR  ^= LED3_MASK)
/** @} */

/**
 * @brief User button
 */
#define BTN_B1_PIN          GPIO_PIN(PORT_A, 0)

/**
 * @brief Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */
/** @} */
