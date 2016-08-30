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

#ifndef NUM_PACKETS
#define NUM_PACKETS 50
#endif

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];


static int rx_counter=0;

static gnrc_netreg_entry_t server = { NULL, GNRC_NETREG_DEMUX_CTX_ALL, KERNEL_PID_UNDEF };


static void start_server(char *port_str)
{
    uint16_t port;

    /* check if server is already running */
    if (server.pid != KERNEL_PID_UNDEF) {
        printf("Error: server already running on port %" PRIu32 "\n",
               server.demux_ctx);
        return;
    }
    /* parse port */
    port = (uint16_t)atoi(port_str);
    if (port == 0) {
        puts("Error: invalid port specified");
        return;
    }
    /* start server (which means registering pktdump for the chosen port) */
    server.pid = thread_getpid();
    server.demux_ctx = (uint32_t)port;
    gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);
    printf("Success: started UDP server on port %" PRIu16 "\n", port);
}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    msg_t msg, reply;

    puts("RIOT network stack example application");

    bool state = 1;
    gnrc_netapi_set(7, NETOPT_RX_END_IRQ, 0, &state,
                    sizeof(state));

    gnrc_rpl_init(7);

    ipv6_addr_t my_addr;

    ipv6_addr_from_str(&my_addr, "2001::1");

    gnrc_ipv6_netif_add_addr(7, &my_addr, 64, 0);

    gnrc_rpl_root_init(1, &my_addr, false, false);

    start_server("1234");

    reply.type = GNRC_NETAPI_MSG_TYPE_ACK;

    while (1) {
        gnrc_pktsnip_t *packet;
        msg_receive(&msg);

        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:

                packet = (gnrc_pktsnip_t *)msg.content.ptr;

                printf(" rx %i counter %i\n", atoi((char *)packet->data), rx_counter++);

                gnrc_pktbuf_release(packet);

                break;

            case GNRC_NETAPI_MSG_TYPE_SND:
                break;

            case GNRC_NETAPI_MSG_TYPE_GET:
            case GNRC_NETAPI_MSG_TYPE_SET:
                reply.content.value = -ENOTSUP;
                msg_reply(&msg, &reply);
                break;

            default:
                break;
        }
    }

    return 0;
}
