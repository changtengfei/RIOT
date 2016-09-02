/*
 * Copyright (C) 2016 HAW Hamburg
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
 * @brief       Example application measuring energy consumption
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
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
#include "board.h"

#ifndef PACKET_DELAY
#define PACKET_DELAY 1000000
#endif
#ifndef NUM_PACKETS
#define NUM_PACKETS 100
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

    LED0_OFF;
    LED1_OFF;
    LED2_OFF;

    // send one packet more (for receiver) but turn on LED after num packets
    for (unsigned int i = 0; i < num+1; i++) {

        sprintf(data, "%02d Msg buffer w/20b", i);

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

        // turn on led after the before last packet
        // wait for the delay. otherwise it's not fair 
        // for the receiver
        if (i == num-1) {
            LED0_ON;
            LED1_ON;
            LED2_ON;
        }
    }

    printf("Sender done");

}

int main(void)
{
    LED0_ON;
    LED1_ON;
    LED2_ON;
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    kernel_pid_t ifs[GNRC_NETIF_NUMOF];
    gnrc_netif_get(ifs);

    puts("RIOT network stack example application");

    gnrc_rpl_init(ifs[0]);

    xtimer_sleep(20);

    char data[21];

    send("2001::1", "1234", data, NUM_PACKETS, PACKET_DELAY);

    /* should be never reached */
    return 0;
}
