/*
 * Copyright (C) 2017 HAW Hamburg
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
 * @brief   Auto initialization for Semtech sx127x network interfaces
 *
 * @author  Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifdef MODULE_SX127X

#include "log.h"
#include "board.h"
#include "net/gnrc.h"
#include "net/gnrc/netif/raw.h"

#include "sx127x.h"
#include "sx127x_params.h"

/**
 * @brief   Define stack parameters for the MAC layer thread
 * @{
 */
#define SX127X_MAC_STACKSIZE     (THREAD_STACKSIZE_DEFAULT)
#ifndef SX127X_MAC_PRIO
#define SX127X_MAC_PRIO          (GNRC_NETIF_PRIO)
#endif

#define SX127X_NUM (sizeof(sx127x_params) / sizeof(sx127x_params[0]))

static sx127x_t sx127x_devs[SX127X_NUM];
static char _sx127x_stacks[SX127X_NUM][SX127X_MAC_STACKSIZE];

void auto_init_sx127x(void)
{
    for (unsigned i = 0; i < SX127X_NUM; i++) {
        LOG_DEBUG("[auto_init_netif] initializing sx127x #%u\n", i);

        sx127x_setup(&sx127x_devs[i], &sx127x_params[i]);

        gnrc_netif_raw_create(_sx127x_stacks[i],
                              SX127X_MAC_STACKSIZE,
                              SX127X_MAC_PRIO, "sx127x",
                              (netdev_t *)&sx127x_devs[i]);

    }
}
#else
typedef int dont_be_pedantic;
#endif /* MODULE_SX127X */

/** @} */
