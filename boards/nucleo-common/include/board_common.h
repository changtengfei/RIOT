/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_nucleo STM Nucleo boards
 * @ingroup     boards
 * @brief       STM Nucleo boards
 */

/**
 * @defgroup    boards_nucleo64_common STM Nucleo 64 boards common
 * @ingroup     boards_nucleo
 * @brief       Common files for STM Nucleo 64 boards
 * @{
 *
 * @file
 * @brief       Common pin definitions and board configuration options
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef BOARD_COMMON_H
#define BOARD_COMMON_H

#include "cpu.h"
#include "periph_conf.h"
#include "arduino_pinmap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   LED pin definitions and handlers
 * @{
 */
#ifdef CPU_MODEL_STM32F302R8
#define LED0_PIN            GPIO_PIN(1, 13)
#define LED0_MASK           (1 << 13)
#else
#define LED0_PIN            GPIO_PIN(2, 13)
#define LED0_MASK           (1 << 13)
#define LED1_PIN            GPIO_PIN(2, 14)
#define LED1_MASK           (1 << 14)
#define LED2_PIN            GPIO_PIN(2, 15)
#define LED2_MASK           (1 << 15)
#endif

#define LED0_ON             (GPIOC->BSRR  = LED0_MASK)
#define LED0_OFF            (GPIOC->BSRR  = (LED0_MASK << 16))
#define LED0_TOGGLE         (GPIOC->ODR  ^= LED0_MASK)

#define LED1_ON             (GPIOC->BSRR  = LED1_MASK)
#define LED1_OFF            (GPIOC->BSRR  = (LED1_MASK << 16))
#define LED1_TOGGLE         (GPIOC->ODR  ^= LED1_MASK)

#define LED2_ON             (GPIOC->BSRR  = LED2_MASK)
#define LED2_OFF            (GPIOC->BSRR  = (LED2_MASK << 16))
#define LED2_TOGGLE         (GPIOC->ODR  ^= LED2_MASK)
/** @} */

/**
 * @brief   User button
 * @{
 */
#define BTN0_PIN            GPIO_PIN(2, 13)
#define BTN0_MODE           GPIO_IN_PU
/** @} */

/**
 * @brief   Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_COMMON_H */
/** @} */
