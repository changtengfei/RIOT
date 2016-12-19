/*
 * Copyright (C) 2015 HAW Hamburg
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
#include "net/gnrc/nomac.h"
#include "net/gnrc.h"

#include "l2_reflector.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Define stack parameters for the MAC layer thread
 * @{
 */
#define L2_REFLECTOR_MAC_STACKSIZE     (THREAD_STACKSIZE_DEFAULT)
#define L2_REFLECTOR_MAC_PRIO          (THREAD_PRIORITY_MAIN - 4)

#define L2_REFLECTOR_NUM (1)

static l2_reflector_t l2_reflector_devs[L2_REFLECTOR_NUM];
static char _nomac_stacks[L2_REFLECTOR_MAC_STACKSIZE][L2_REFLECTOR_NUM];

void auto_init_l2_reflector(void)
{
    for (int i = 0; i < L2_REFLECTOR_NUM; i++) {

        l2_reflector_init(&l2_reflector_devs[i]);
        gnrc_nomac_init(_nomac_stacks[i],
                        L2_REFLECTOR_MAC_STACKSIZE, L2_REFLECTOR_MAC_PRIO,
                        "l2_reflector", (gnrc_netdev_t *)&l2_reflector_devs[i]);
    }
}
#else
typedef int dont_be_pedantic;
#endif /* MODULE_L2_REFLECTOR */

/** @} */
