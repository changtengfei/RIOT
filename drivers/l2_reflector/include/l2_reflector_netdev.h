/*
 * Copyright (C) 2015 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_l2_reflector
 * @{
 *
 * @file
 * @brief       Netdev interface to l2_reflector drivers
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef L2_REFLECTOR_NETDEV_H_
#define L2_REFLECTOR_NETDEV_H_

#include "net/gnrc/netdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Reference to the netdev device driver struct
 */
extern const gnrc_netdev_driver_t l2_reflector_driver;

#ifdef __cplusplus
}
#endif

#endif /* L2_REFLECTOR_NETDEV_H_ */
/** @} */
