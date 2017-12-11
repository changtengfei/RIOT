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

#include "shell.h"
#include "msg.h"
#include "random.h"

#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/ipv6/nib.h"
#include "net/gnrc/pktdump.h"
#include "net/gnrc.h"

#ifdef NODE_GW
#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
#endif

#ifdef MODULE_GNRC_UDP
extern int udp_cmd(int argc, char **argv);

static const shell_command_t shell_commands[] = {
    { "udp", "send data over UDP and listen on UDP ports", udp_cmd },
    { NULL, NULL, NULL }
};
#endif

int main(void)
{
#ifdef NODE_LORA
    // IP addys
    char *addr_string="abcd::1";
    ipv6_addr_t addr, addr2;
    uint16_t flags = GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID;

    uint8_t prefix_len = 128;
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    flags |= (prefix_len << 8U);
    if (gnrc_netapi_set(5, NETOPT_IPV6_ADDR, flags, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to add IPv6 address\n");
        return 1;
    }

    addr_string="fe80::";
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    if (gnrc_netapi_set(5, NETOPT_IPV6_ADDR_REMOVE, 0, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to remove IPv6 address\n");
        return 1;
    }

    addr_string="fe80::1";
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    if (gnrc_netapi_set(5, NETOPT_IPV6_ADDR, flags, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to add IPv6 address\n");
        return 1;
    }

    // neigh to GW
    addr_string="fe80::2";
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    uint8_t l2addr[2]={0xff, 0xff};
    gnrc_ipv6_nib_nc_set(&addr, 5, l2addr, 2);

    // route to linux/GW
    char *  addr_string_global="fd07::1234";
    if (ipv6_addr_from_str(&addr, addr_string_global) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }

    char * addr_string_next="fe80::2";
    if (ipv6_addr_from_str(&addr2, addr_string_next) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    gnrc_ipv6_nib_ft_add(&addr, 64, &addr2, 5, 0);


    // application
    uint32_t cnt=0;
    while(1){

        int iface = 5;
        uint16_t port=8888;
        char data[20];

        sprintf(data, "LORA,%" PRIu32"\n", cnt);
        printf("%s\n", data);


        // parse destination address
        if (ipv6_addr_from_str(&addr, addr_string_global) == NULL) {
            puts("Error: unable to parse destination address");
            return 1;
        }

        gnrc_pktsnip_t *payload, *udp, *ip;
        // allocate payload 
        payload = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF);
        if (payload == NULL) {
            puts("Error: unable to copy data to packet buffer");
            return 1;
        }

        // allocate UDP header, set source port := destination port 
        udp = gnrc_udp_hdr_build(payload, port, port);
        if (udp == NULL) {
            puts("Error: unable to allocate UDP header");
            gnrc_pktbuf_release(payload);
            return 1;
        }
        // allocate IPv6 header
        ip = gnrc_ipv6_hdr_build(udp, NULL, &addr);
        if (ip == NULL) {
            puts("Error: unable to allocate IPv6 header");
            gnrc_pktbuf_release(udp);
            return 1;
        }
        // add netif header, if interface was given
        if (iface > 0) {
            gnrc_pktsnip_t *netif = gnrc_netif_hdr_build(NULL, 0, NULL, 0);

            ((gnrc_netif_hdr_t *)netif->data)->if_pid = (kernel_pid_t)iface;
            LL_PREPEND(ip, netif);
        }
        // send packet
        if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
            puts("Error: unable to locate UDP thread");
            gnrc_pktbuf_release(ip);
            return 1;
        }

        puts("Sent UDP packet");
        cnt++;
        //xtimer_usleep(random_uint32_range(1750000, 2250000));
        //xtimer_usleep(random_uint32_range(200000, 300000));
        xtimer_usleep(random_uint32_range(90000, 110000));
        //xtimer_usleep(random_uint32_range(20000, 30000));
    }

#endif

#ifdef NODE_802154
    // global addy
    char *addr_string="affe::1";
    ipv6_addr_t addr, addr2;
    uint16_t flags = GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID;

    uint8_t prefix_len = 128;
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    flags |= (prefix_len << 8U);
    if (gnrc_netapi_set(6, NETOPT_IPV6_ADDR, flags, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to add IPv6 address\n");
        return 1;
    }

    // route to linux/GW
    char *  addr_string_global="fd07::1234";
    if (ipv6_addr_from_str(&addr, addr_string_global) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }

    char * addr_string_next="fe80::1617:1a10:6136:4e02";
    if (ipv6_addr_from_str(&addr2, addr_string_next) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    gnrc_ipv6_nib_ft_add(&addr, 64, &addr2, 6, 0);


    // application
    uint32_t cnt=0;
    while(1){

        int iface = 6;
        uint16_t port=8888;
        char data[20];

        sprintf(data, "802154,%" PRIu32"\n", cnt);
        printf("%s\n", data);


        // parse destination address
        if (ipv6_addr_from_str(&addr, addr_string_global) == NULL) {
            puts("Error: unable to parse destination address");
            return 1;
        }

        gnrc_pktsnip_t *payload, *udp, *ip;
        // allocate payload 
        payload = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF);
        if (payload == NULL) {
            puts("Error: unable to copy data to packet buffer");
            return 1;
        }

        // allocate UDP header, set source port := destination port 
        udp = gnrc_udp_hdr_build(payload, port, port);
        if (udp == NULL) {
            puts("Error: unable to allocate UDP header");
            gnrc_pktbuf_release(payload);
            return 1;
        }
        // allocate IPv6 header
        ip = gnrc_ipv6_hdr_build(udp, NULL, &addr);
        if (ip == NULL) {
            puts("Error: unable to allocate IPv6 header");
            gnrc_pktbuf_release(udp);
            return 1;
        }
        // add netif header, if interface was given
        if (iface > 0) {
            gnrc_pktsnip_t *netif = gnrc_netif_hdr_build(NULL, 0, NULL, 0);

            ((gnrc_netif_hdr_t *)netif->data)->if_pid = (kernel_pid_t)iface;
            LL_PREPEND(ip, netif);
        }
        // send packet
        if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
            puts("Error: unable to locate UDP thread");
            gnrc_pktbuf_release(ip);
            return 1;
        }

        puts("Sent UDP packet");
        cnt++;
        //xtimer_usleep(random_uint32_range(1750000, 2250000));
        //xtimer_usleep(random_uint32_range(200000, 300000));
        xtimer_usleep(random_uint32_range(90000, 110000));
        //xtimer_usleep(random_uint32_range(20000, 30000));
    }

#endif


#ifdef NODE_GW

    // remove unneeded LoRa addys
    char *addr_string="fe80::";
    ipv6_addr_t addr, addr2;
    uint16_t flags = GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID;

    addr_string="fe80::";
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    if (gnrc_netapi_set(8, NETOPT_IPV6_ADDR_REMOVE, 0, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to remove IPv6 address\n");
        return 1;
    }
/*    // global addy
    addr_string="cafe::1";
    uint8_t prefix_len = 128;
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    flags |= (prefix_len << 8U);
    if (gnrc_netapi_set(6, NETOPT_IPV6_ADDR, flags, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to add IPv6 address\n");
        return 1;
    }
*/
    // local addy for LoRa
    addr_string="fe80::2";
    uint8_t prefix_len = 128;
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    flags |= (prefix_len << 8U);
    if (gnrc_netapi_set(8, NETOPT_IPV6_ADDR, flags, &addr,
                        sizeof(addr)) < 0) {
        printf("error: unable to add IPv6 address\n");
        return 1;
    }
    // for safety, neigh for LoRa
    addr_string="fe80::1";
    if (ipv6_addr_from_str(&addr, addr_string) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    uint8_t l2addr[2]={0xff, 0xff};
    gnrc_ipv6_nib_nc_set(&addr, 8, l2addr, 2);

    // route to linux
    char *  addr_string_global="fd07::1234";
    if (ipv6_addr_from_str(&addr, addr_string_global) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }

    char * addr_string_next="fe80::dcf7:67c6:2ede:d092";
    if (ipv6_addr_from_str(&addr2, addr_string_next) == NULL) {
        puts("error: unable to parse IPv6 address.");
        return 1;
    }
    gnrc_ipv6_nib_ft_add(&addr, 64, &addr2, 7, 0);
    xtimer_sleep(15);
    // set LoRa interface to rx state
    netopt_state_t state = NETOPT_STATE_RX;
    if (gnrc_netapi_set(8, NETOPT_STATE, 0, &state, sizeof(netopt_state_t)) < 0) {
        printf("error: unable to set state to NETOPT_STATE_RX\n");
        return 1;
    }
    else{
        printf("Succesfully set NETOPT_STATE_RX\n");
    }

#endif


/*
    gnrc_netreg_entry_t dump = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                          gnrc_pktdump_pid);
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &dump);*/
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */



#ifdef NODE_GW
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("RIOT network stack example application");

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
#ifdef MODULE_GNRC_UDP
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
#else
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
#endif

#endif
    /* should be never reached */
    return 0;
}
