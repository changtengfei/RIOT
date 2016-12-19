/*
 * Copyright (C) 2015 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    drivers_l2_reflector L2 REFLECTOR based drivers
 * @ingroup     drivers_netdev
 *
 * This module contains drivers for radio devices in Atmel's AT86RF2xx series.
 * The driver is aimed to work with all devices of this series.
 *
 * @{
 *
 * @file
 * @brief       Interface definition for AT86RF2xx based drivers
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef L2_REFLECTOR_H_
#define L2_REFLECTOR_H_

#include <stdint.h>

#include "board.h"
#include "net/gnrc/netdev.h"
#include "l2_reflector.h"
//#include "l2_reflector_netdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum possible packet size in byte
 */
#define L2_REFLECTOR_MAX_PKT_LENGTH        (127)

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
 * @brief   Device descriptor for l2 reflector devices
 */
typedef struct {
    /* netdev fields */
    const gnrc_netdev_driver_t *driver; /**< pointer to the devices interface */
    gnrc_netdev_event_cb_t event_cb;    /**< netdev event callback */
    kernel_pid_t mac_pid;               /**< the driver's thread's PID */
    gnrc_nettype_t proto;               /**< protocol the radio expects */
    uint8_t seq_nr;                     /**< sequence number to use next */
    uint8_t frame_len;                  /**< length of the current TX frame */
    uint16_t pan;                       /**< currently used PAN ID */
    uint8_t chan;                       /**< currently used channel */
    uint8_t addr_short[2];              /**< the radio's short address */
    uint8_t addr_long[8];               /**< the radio's long address */
    uint16_t options;                   /**< state of used options */
} l2_reflector_t;


/**
 * @brief   Initialize a given L2_REFLECTOR device
 *
 * @param[out] dev          device descriptor
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int l2_reflector_init(l2_reflector_t *dev);


/**
 * @brief   Set the short address of the given device
 *
 * @param[in] dev           device to write to
 * @param[in] addr          (2-byte) short address to set
 */
void l2_reflector_set_addr_short(l2_reflector_t *dev, uint16_t addr);

/**
 * @brief   Set the long address of the given device
 *
 * @param[in] dev           device to write to
 * @param[in] addr          (8-byte) long address to set
 */
void l2_reflector_set_addr_long(l2_reflector_t *dev, uint64_t addr);

/**
 * @brief   Get the short address of the given device
 *
 * @param[in] dev           device to read from
 *
 * @return                  the currently set (2-byte) short address
 */
uint16_t l2_reflector_get_addr_short(l2_reflector_t *dev);

/**
 * @brief   Get the configured long address of the given device
 *
 * @param[in] dev           device to read from
 *
 * @return                  the currently set (8-byte) long address
 */
uint64_t l2_reflector_get_addr_long(l2_reflector_t *dev);


#ifdef __cplusplus
}
#endif

#endif /* L2_REFLECTOR_H_ */
/** @} */
