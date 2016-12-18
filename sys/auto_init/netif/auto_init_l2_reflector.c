/*
 * Copyright (C) 2016 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/*
 * @ingroup auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief   Auto initialization for l2 reflector network interfaces
 *
 * @author  Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifdef MODULE_L2_REFLECTOR

#include "board.h"
#include "net/gnrc/netdev2.h"
#include "net/gnrc/netdev2/ieee802154.h"
#include "net/gnrc.h"

#include "l2_reflector.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Define stack parameters for the MAC layer thread
 * @{
 */
#define L2_REFLECTOR_MAC_STACKSIZE     (THREAD_STACKSIZE_DEFAULT)
#ifndef L2_REFLECTOR_MAC_PRIO
#define L2_REFLECTOR_MAC_PRIO           (GNRC_NETDEV2_MAC_PRIO)
#endif


static l2_reflector_t l2_reflector_dev;
static gnrc_netdev2_t gnrc_adpt;
static char _l2_reflector_stacks[L2_REFLECTOR_MAC_STACKSIZE];

void auto_init_l2_reflector(void)
{
    DEBUG("### auto_init_l2_reflector ###");

    l2_reflector_setup(&l2_reflector_dev);
    int res = gnrc_netdev2_ieee802154_init(&gnrc_adpt,
                                       (netdev2_ieee802154_t *)&l2_reflector_dev);

    if (res < 0) {
        DEBUG("Error initializing l2_reflector radio device!\n");
    }
    else {
        gnrc_netdev2_init(_l2_reflector_stacks,
                          L2_REFLECTOR_MAC_STACKSIZE,
                          L2_REFLECTOR_MAC_PRIO,
                          "l2_reflector",
                          &gnrc_adpt);
    }
}
#else
typedef int dont_be_pedantic;
#endif /* MODULE_L2_REFLECTOR */

/** @} */
