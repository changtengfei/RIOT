/*
 * Copyright (C) 2016 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     l2_reflector
 * @{
 *
 * @file
 * @brief       Netdev interface to L2_REFLECTOR drivers
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef L2_REFLECTOR_NETDEV_H_
#define L2_REFLECTOR_NETDEV_H_

#include "net/netdev2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Reference to the netdev device driver struct
 */
extern const netdev2_driver_t l2_reflector_driver;

#ifdef __cplusplus
}
#endif

#endif /* L2_REFLECTOR_NETDEV_H_ */
/** @} */
