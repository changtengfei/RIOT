/*
 * Copyright (C) 2015, 2016  INRIA.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 * @brief       Shell commands to interact with the CCN-Lite stack
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 *
 * @}
 */

#include "random.h"
#include "sched.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/pktdump.h"
#include "ccn-lite-riot.h"
#include "ccnl-pkt-ndntlv.h"
#include "board.h"

#define BUF_SIZE (64)

/**
 * Maximum number of Interest retransmissions
 */
#define CCNL_INTEREST_RETRIES   (3)

#define MAX_ADDR_LEN            (8U)

int rx_counter=0;
int snd_cont_counter=0;

extern struct ccnl_relay_s ccnl_relay;
extern struct ccnl_buf_s *bufCleanUpList;

static unsigned char _int_buf[BUF_SIZE];

static const char *_default_content = "Start the RIOT!";
static unsigned char _out[CCNL_MAX_PACKET_SIZE];

static gnrc_netreg_entry_t chunk_dump = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                               KERNEL_PID_UNDEF);

/* usage for open command */
static void _open_usage(void)
{
    puts("ccnl <interface>");
}

int _ccnl_open(int argc, char **argv)
{
    /* check if already running */
    if (ccnl_relay.ifcount >= CCNL_MAX_INTERFACES) {
        puts("Already opened max. number of interfaces for CCN!");
        return -1;
    }

    /* check if parameter is given */
    if (argc != 2) {
        _open_usage();
        return -1;
    }

    /* check if given number is a valid netif PID */
    int pid = atoi(argv[1]);
    if (!gnrc_netif_exist(pid)) {
        printf("%i is not a valid interface!\n", pid);
        return -1;
    }

    ccnl_start();

    /* set the relay's PID, configure the interface to interface to use CCN
     * nettype */
    if (ccnl_open_netif(pid, GNRC_NETTYPE_CCN) < 0) {
        puts("Error registering at network interface!");
        return -1;
    }

    return 0;
}


static void _content_usage(char *argv)
{
    printf("usage: %s <URI> [content]\n"
            "%% %s /riot/peter/schmerzl             (default content)\n"
            "%% %s /riot/peter/schmerzl RIOT\n",
            argv, argv, argv);
}

int _ccnl_content(int argc, char **argv)
{
    char *body = (char*) _default_content;
    int arg_len = strlen(_default_content) + 1;
    int offs = CCNL_MAX_PACKET_SIZE;
    if (argc < 2) {
        _content_usage(argv[0]);
        return -1;
    }

    if (argc > 2) {
        char buf[BUF_SIZE];
        memset(buf, ' ', BUF_SIZE);
        char *buf_ptr = buf;
        for (int i = 2; (i < argc) && (buf_ptr < (buf + BUF_SIZE)); i++) {
            arg_len = strlen(argv[i]);
            if ((buf_ptr + arg_len) > (buf + BUF_SIZE)) {
                arg_len = (buf + BUF_SIZE) - buf_ptr;
            }
            strncpy(buf_ptr, argv[i], arg_len);
            buf_ptr += arg_len + 1;
        }
        *buf_ptr = '\0';
        body = buf;
        arg_len = strlen(body);
    }

    int suite = CCNL_SUITE_NDNTLV;

    struct ccnl_prefix_s *prefix = ccnl_URItoPrefix(argv[1], suite, NULL, NULL);

    arg_len = ccnl_ndntlv_prependContent(prefix, (unsigned char*) body, arg_len, NULL, NULL, &offs, _out);

    unsigned char *olddata;
    unsigned char *data = olddata = _out + offs;

    int len;
    unsigned typ;

    if (ccnl_ndntlv_dehead(&data, &arg_len, (int*) &typ, &len) ||
        typ != NDN_TLV_Data) {
        return -1;
    }

    struct ccnl_content_s *c = 0;
    struct ccnl_pkt_s *pk = ccnl_ndntlv_bytes2pkt(typ, olddata, &data, &arg_len);
    c = ccnl_content_new(&ccnl_relay, &pk);
    ccnl_content_add2cache(&ccnl_relay, c);
    c->flags |= CCNL_CONTENT_FLAGS_STATIC;

    return 0;
}

static struct ccnl_face_s *_intern_face_get(char *addr_str)
{
    /* initialize address with 0xFF for broadcast */
    uint8_t relay_addr[MAX_ADDR_LEN];
    memset(relay_addr, UINT8_MAX, MAX_ADDR_LEN);
    size_t addr_len = gnrc_netif_addr_from_str(relay_addr, sizeof(relay_addr), addr_str);

    if (addr_len == 0) {
        printf("Error: %s is not a valid link layer address\n", addr_str);
        return NULL;
    }

    sockunion sun;
    sun.sa.sa_family = AF_PACKET;
    memcpy(&(sun.linklayer.sll_addr), relay_addr, addr_len);
    sun.linklayer.sll_halen = addr_len;
    sun.linklayer.sll_protocol = htons(ETHERTYPE_NDN);

    /* TODO: set correct interface instead of always 0 */
    struct ccnl_face_s *fibface = ccnl_get_face_or_create(&ccnl_relay, 0, &sun.sa, sizeof(sun.linklayer));

    return fibface;
}

static int _intern_fib_add(char *pfx, char *addr_str)
{
    int suite = CCNL_SUITE_NDNTLV;
    struct ccnl_prefix_s *prefix = ccnl_URItoPrefix(pfx, suite, NULL, 0);
    if (!prefix) {
        puts("Error: prefix could not be created!");
        return -1;
    }

    struct ccnl_face_s *fibface = _intern_face_get(addr_str);
    if (fibface == NULL) {
        return -1;
    }
    fibface->flags |= CCNL_FACE_FLAGS_STATIC;

    if (ccnl_fib_add_entry(&ccnl_relay, prefix, fibface) != 0) {
        printf("Error adding (%s : %s) to the FIB\n", pfx, addr_str);
        return -1;
    }

    return 0;
}

static void _interest_usage(char *arg)
{
    printf("usage: %s <URI> [relay]\n"
            "%% %s /riot/peter/schmerzl                     (classic lookup)\n",
            arg, arg);
}

int _ccnl_interest(int argc, char **argv)
{
    if (argc < 2) {
        _interest_usage(argv[0]);
        return -1;
    }

    if (argc > 2) {
        if (_intern_fib_add(argv[1], argv[2]) < 0) {
            _interest_usage(argv[0]);
            return -1;
        }
    }

    memset(_int_buf, '\0', BUF_SIZE);

    struct ccnl_prefix_s *prefix = ccnl_URItoPrefix(argv[1], CCNL_SUITE_NDNTLV, NULL, 0);
    int res = ccnl_send_interest(prefix, _int_buf, BUF_SIZE);
    free_prefix(prefix);

    return -1;
}

int _ccnl_start_chunk_dump(int argc, char **argv){
    (void)argc;
    (void)argv;

    /* check if chunk_dump is already running */
    if (chunk_dump.target.pid != KERNEL_PID_UNDEF) {
        puts("Error: chunk_dump already running");
        return -1;
    }

    /* start pktdump */
    chunk_dump.target.pid = gnrc_pktdump_pid;
    gnrc_netreg_register(GNRC_NETTYPE_CCN_CHUNK, &chunk_dump);
    return 0;
}

static void _ccnl_fib_usage(char *argv)
{
    printf("usage: %s [<action> <options>]\n"
           "prints the FIB if called without parameters:\n"
           "%% %s\n"
           "<action> may be one of the following\n"
           "  * \"add\" - adds an entry to the FIB, requires a prefix and a next-hop address, e.g.\n"
           "            %s add /riot/peter/schmerzl ab:cd:ef:01:23:45:67:89\n"
           "  * \"del\" - deletes an entry to the FIB, requires a prefix or a next-hop address, e.g.\n"
           "            %s del /riot/peter/schmerzl\n"
           "            %s del ab:cd:ef:01:23:45:67:89\n",
            argv, argv, argv, argv, argv);
}

int _ccnl_fib(int argc, char **argv)
{
    if (argc < 2) {
        ccnl_fib_show(&ccnl_relay);
    }
    else if ((argc == 3) && (strncmp(argv[1], "del", 3) == 0)) {
        int suite = CCNL_SUITE_NDNTLV;
        if (strchr(argv[2], '/')) {
            struct ccnl_prefix_s *prefix = ccnl_URItoPrefix(argv[2], suite, NULL, 0);
            if (!prefix) {
                puts("Error: prefix could not be created!");
                return -1;
            }
            int res = ccnl_fib_rem_entry(&ccnl_relay, prefix, NULL);
            free_prefix(prefix);
            return res;
        }
        else {
            struct ccnl_face_s *face = _intern_face_get(argv[2]);
            if (face == NULL) {
                printf("There is no face for address %s\n", argv[1]);
                return -1;
            }
            int res = ccnl_fib_rem_entry(&ccnl_relay, NULL, face);
            return res;
        }
    }
    else if ((argc == 4) && (strncmp(argv[1], "add", 3) == 0)) {
        if (_intern_fib_add(argv[2], argv[3]) < 0) {
            _ccnl_fib_usage(argv[0]);
            return -1;
        }
    }
    else {
        _ccnl_fib_usage(argv[0]);
        return -1;
    }
    return 0;
}

int _ccnl_stats(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf(" rx_counter %i\n", rx_counter);
    printf(" snd_cont_counter %i\n", snd_cont_counter);
    return 0;
}

int _leds_on(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    LED0_ON;
    LED1_ON;
    LED2_ON;

    return 0;
}

int _leds_off(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    LED0_OFF;
    LED1_OFF;
    LED2_OFF;

    return 0;
}

int _clean(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct ccnl_relay_s *ccnl = &ccnl_relay;

    printf("ccnl_core_cleanup %p\n", (void *) ccnl);
    while (ccnl->pit)
        ccnl_interest_remove(ccnl, ccnl->pit);
    while (ccnl->contents)
        ccnl_content_remove(ccnl, ccnl->contents);
    while (ccnl->nonces) {
        struct ccnl_buf_s *tmp = ccnl->nonces->next;
        ccnl_free(ccnl->nonces);
        ccnl->nonces = tmp;
    }
    while (bufCleanUpList) {
        struct ccnl_buf_s *tmp = bufCleanUpList->next;
        ccnl_free(bufCleanUpList);
        bufCleanUpList = tmp;
    }

    return 0;
}