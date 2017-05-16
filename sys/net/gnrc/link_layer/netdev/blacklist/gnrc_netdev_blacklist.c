/*
 * Copyright (C) 2017 Hamburg University of Applied Sciences
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#include <stdio.h>

#include "bitfield.h"

#include "net/gnrc/netdev.h"
#include "net/gnrc/netdev/blacklist.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define _MAX_L2_ADDR_LEN    (8U)

uint8_t gnrc_netdev_blacklist[GNRC_NETDEV_BLACKLIST_SIZE * _MAX_L2_ADDR_LEN];
BITFIELD(gnrc_netdev_blacklist_set, GNRC_NETDEV_BLACKLIST_SIZE);

#if ENABLE_DEBUG
static char addr_str[_MAX_L2_ADDR_LEN * 3];
#endif

int gnrc_netdev_blacklist_add(const uint8_t *addr, size_t addr_len)
{
    for (int i = 0; i < GNRC_NETDEV_BLACKLIST_SIZE; i++) {
        if (!bf_isset(gnrc_netdev_blacklist_set, i)) {
            bf_set(gnrc_netdev_blacklist_set, i);
            memcpy(&gnrc_netdev_blacklist[i], addr, addr_len);
            DEBUG("Hardware address blacklist: blacklisted %s\n",
                  gnrc_netif_addr_to_str(addr_str, sizeof(addr_str),
                    addr, addr_len));
            return 0;
        }
    }
    return -1;
}

void gnrc_netdev_blacklist_del(const uint8_t *addr, size_t addr_len)
{
    for (int i = 0; i < GNRC_NETDEV_BLACKLIST_SIZE; i++) {
        if (memcmp(addr, &gnrc_netdev_blacklist[i], addr_len)) {
            bf_unset(gnrc_netdev_blacklist_set, i);
            DEBUG("Hardware address blacklist: unblacklisted %s\n",
                  gnrc_netif_addr_to_str(addr_str, sizeof(addr_str),
                    addr, addr_len));
        }
    }
}

bool gnrc_netdev_blacklisted(const uint8_t *addr, size_t addr_len)
{
    for (int i = 0; i < GNRC_NETDEV_BLACKLIST_SIZE; i++) {
        if (bf_isset(gnrc_netdev_blacklist_set, i) &&
            memcmp(addr, &gnrc_netdev_blacklist[i], addr_len)) {
            return true;
        }
    }
    return false;
}

void gnrc_netdev_blacklist_print(void)
{
    for (int i = 0; i < GNRC_NETDEV_BLACKLIST_SIZE; i++) {
        if (bf_isset(gnrc_netdev_blacklist_set, i)) {
            puts(gnrc_netif_addr_to_str(addr_str, sizeof(addr_str), 
                &gnrc_netdev_blacklist[i], 6));
        }
    }
}

/** @} */
