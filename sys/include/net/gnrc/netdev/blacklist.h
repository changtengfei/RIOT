/*
 * Copyright (C) 2017 Hamburg University of Applied Sciences
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_netdev_blacklist netdev hardware address blacklist
 * @ingroup     net_gnrc_netdev
 * @brief       Module to blacklist hardware addresses
 * @{
 *
 * @file
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef GNRC_NETDEV_BLACKLIST_H
#define GNRC_NETDEV_BLACKLIST_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Maximum size of the blacklist.
 */
#ifndef GNRC_NETDEV_BLACKLIST_SIZE
#define GNRC_NETDEV_BLACKLIST_SIZE    (8)
#endif

/**
 * @brief   Adds a hardware address to the blacklist.
 *
 * @param[in] addr      A hardware address.
 * @param[in] addr_len  Length of @p addr
 *
 * @return  0, on success.
 * @return  -1, if blacklist is full.
 */
int gnrc_netdev_blacklist_add(const uint8_t *addr, size_t addr_len);

/**
 * @brief   Removes an hardware address from the blacklist.
 *
 * Addresses not in the blacklist will be ignored.
 *
 * @param[in] addr      A hardware address.
 * @param[in] addr_len  Length of @p addr
 */
void gnrc_netdev_blacklist_del(const uint8_t *addr, size_t addr_len);

/**
 * @brief   Checks if an hardware address is blacklisted.
 *
 * @param[in] addr      A hardware address.
 * @param[in] addr_len  Length of @p addr
 *
 * @return  true, if @p addr is blacklisted.
 * @return  false, if @p addr is not blacklisted.
 */
bool gnrc_netdev_blacklisted(const uint8_t *addr, size_t addr_len);

/**
 * @brief   Prints the blacklist.
 */
void gnrc_netdev_blacklist_print(void);


#ifdef __cplusplus
}
#endif

#endif /* GNRC_NETDEV_BLACKLIST_H */
/** @} */