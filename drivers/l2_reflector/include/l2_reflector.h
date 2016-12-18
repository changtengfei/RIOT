/*
 * Copyright (C) 2016 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    drivers_l2_reflector
 * @ingroup     drivers_netdev_netdev2
 *
 * @{
 *
 * @file
 * @brief       Interface definition for L2_REFLECTOR based drivers
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef L2_REFLECTOR_H_
#define L2_REFLECTOR_H_

#include <stdint.h>

#include "board.h"
#include "periph/spi.h"
#include "periph/gpio.h"
#include "net/netdev2.h"
#include "net/netdev2/ieee802154.h"
#include "net/gnrc/nettype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum possible packet size in byte
 */
#define L2_REFLECTOR_MAX_PKT_LENGTH        (IEEE802154_FRAME_LEN_MAX)

/**
 * @brief   Default addresses used if the CPUID module is not present
 * @{
 */
#define L2_REFLECTOR_DEFAULT_ADDR_SHORT    (0x0230)
#define L2_REFLECTOR_DEFAULT_ADDR_LONG     (0x1222334455667788)
/** @} */

/**
 * @brief   Internal device option flags
 * @{
 */
#define L2_REFLECTOR_OPT_AUTOACK        (0x0001)       /**< auto ACKs active */
#define L2_REFLECTOR_OPT_SRC_ADDR_LONG  (0x0200)       /**< send data using long
                                                        *   source address */
#define L2_REFLECTOR_OPT_USE_SRC_PAN    (0x0400)       /**< do not compress source
                                                        *   PAN ID */
/** @} */

/**
 * @brief   Initialize a given L2_REFLECTOR device
 *
 * @param[out] dev          device descriptor
 *
 */
void l2_reflector_setup(l2_reflector_t *dev);


#ifdef __cplusplus
}
#endif

#endif /* L2_REFLECTOR_H_ */
/** @} */
