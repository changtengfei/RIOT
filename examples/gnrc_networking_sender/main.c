/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdbool.h>

#include "shell.h"
#include "msg.h"
#include "net/gnrc.h"
#include "net/ipv6.h"
#include "net/gnrc/rpl.h"
#include "net/gnrc/udp.h"
#include "timex.h"
#include "xtimer.h"
#include "ps.h"

#ifndef PACKET_DELAY
#define error "PACKET_DELAY not defined"
#endif
#ifndef NUM_PACKETS
#define error "NUM_PACKETS not defined"
#endif


#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];


static void send(char *addr_str, char *port_str, char *data, unsigned int num,
                 unsigned int delay)
{
    uint16_t port;
    ipv6_addr_t addr;


    /* parse destination address */
    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        puts("Error: unable to parse destination address");
        return;
    }
    /* parse port */
    port = (uint16_t)atoi(port_str);
    if (port == 0) {
        puts("Error: unable to parse destination port");
        return;
    }

    for (unsigned int i = 0; i < num; i++) {

        printf(" tx (%i)\n", i);

        gnrc_pktsnip_t *payload, *udp, *ip;

        /* allocate payload */
        payload = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF);
        if (payload == NULL) {
            puts("Error: unable to copy data to packet buffer");
            return;
        }
        /* allocate UDP header, set source port := destination port */
        udp = gnrc_udp_hdr_build(payload, port, port);
        if (udp == NULL) {
            puts("Error: unable to allocate UDP header");
            gnrc_pktbuf_release(payload);
            return;
        }
        /* allocate IPv6 header */
        ip = gnrc_ipv6_hdr_build(udp, NULL, &addr);
        if (ip == NULL) {
            puts("Error: unable to allocate IPv6 header");
            gnrc_pktbuf_release(udp);
            return;
        }
        /* send packet */
        if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
            puts("Error: unable to locate UDP thread");
            gnrc_pktbuf_release(ip);
            return;
        }

        xtimer_usleep(delay);
    }

}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    /*msg_t msg;*/

    puts("RIOT network stack example application");

    gnrc_rpl_init(7);

    xtimer_sleep(20);

    char data[21];

    sprintf(data, "Msg %02d buffer w/20b", (unsigned int)666);

    send("2001::1", "1234", data, NUM_PACKETS, PACKET_DELAY);

    /* should be never reached */
    return 0;
}
