/*
 * Copyright (C) 2015 HAW Hamburg 
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_l2_reflector
 * @{
 *
 * @file
 * @brief       Implementation of public functions for l2 reflectors
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/uio.h>

#include "xtimer.h"
#include "periph/cpuid.h"
#include "byteorder.h"
#include "net/ieee802154.h"
#include "net/gnrc.h"
#include "l2_reflector.h"
#include "l2_reflector_netdev.h"


#define ENABLE_DEBUG (0)
#include "debug.h"

#define _MAX_MHR_OVERHEAD   (25)

int l2_reflector_init(l2_reflector_t *dev) {

    dev->driver = &l2_reflector_driver;

#if CPUID_LEN
/* make sure that the buffer is always big enough to store a 64bit value */
#   if CPUID_LEN < 8
    uint8_t cpuid[8];
#   else
    uint8_t cpuid[CPUID_LEN];
#endif
    eui64_t addr_long;
#endif

#if CPUID_LEN
    /* in case CPUID_LEN < 8, fill missing bytes with zeros */
    memset(cpuid, 0, CPUID_LEN);

    cpuid_get(cpuid);

#if CPUID_LEN > 8
    for (int i = 8; i < CPUID_LEN; i++) {
        cpuid[i & 0x07] ^= cpuid[i];
    }
#endif
    /* make sure we mark the address as non-multicast and not globally unique */
    cpuid[0] &= ~(0x01);
    cpuid[0] |= 0x02;
    
    /* copy and set long address */
    memcpy(&addr_long, cpuid, 8);
#endif

    l2_reflector_set_addr_long(dev, NTOHLL(addr_long.uint64.u64));
    l2_reflector_set_addr_short(dev, NTOHS(addr_long.uint16[0].u16));


    /* reset options and sequence number */
    dev->seq_nr = 0;
    dev->options = 0;

    dev->options |= L2_REFLECTOR_OPT_AUTOACK;

#ifdef MODULE_GNRC_SIXLOWPAN
    dev->proto = GNRC_NETTYPE_SIXLOWPAN;
#else
    dev->proto = GNRC_NETTYPE_UNDEF;
#endif

    DEBUG("[l2_reflector]: init complete.\n");

    return 0;
}

void l2_reflector_set_addr_short(l2_reflector_t *dev, uint16_t addr)
{
    dev->addr_short[0] = addr >> 8;
    dev->addr_short[1] = addr & 0xff;
}

void l2_reflector_set_addr_long(l2_reflector_t *dev, uint64_t addr)
{
    for (int i = 0; i < 8; i++) {
        dev->addr_long[i] = (addr >> ((7 - i) * 8));
    }
}

uint16_t l2_reflector_get_addr_short(l2_reflector_t *dev)
{
    return (dev->addr_short[0] << 8) | dev->addr_short[1];
}

uint64_t l2_reflector_get_addr_long(l2_reflector_t *dev)
{
    uint64_t addr;
    uint8_t *ap = (uint8_t *)(&addr);
    for (int i = 0; i < 8; i++) {
        ap[i] = dev->addr_long[7 - i];
    }
    return addr;
}

static size_t _make_data_frame_hdr(l2_reflector_t *dev, uint8_t *buf,
                                   gnrc_netif_hdr_t *hdr)
{
    int pos = 0;

    /* we are building a data frame here */
    buf[0] = IEEE802154_FCF_TYPE_DATA;
    buf[1] = IEEE802154_FCF_VERS_V1;

    /* if AUTOACK is enabled, then we also expect ACKs for this packet */
    if (!(hdr->flags & GNRC_NETIF_HDR_FLAGS_BROADCAST) &&
        !(hdr->flags & GNRC_NETIF_HDR_FLAGS_MULTICAST) &&
        (dev->options & L2_REFLECTOR_OPT_AUTOACK)) {
        buf[0] |= IEEE802154_FCF_ACK_REQ;
    }

    /* fill in destination PAN ID */
    pos = 3;
    buf[pos++] = (uint8_t)((dev->pan) & 0xff);
    buf[pos++] = (uint8_t)((dev->pan) >> 8);

    /* fill in destination address */
    if (hdr->flags &
        (GNRC_NETIF_HDR_FLAGS_BROADCAST | GNRC_NETIF_HDR_FLAGS_MULTICAST)) {
        buf[1] |= IEEE802154_FCF_DST_ADDR_SHORT;
        buf[pos++] = 0xff;
        buf[pos++] = 0xff;
    }
    else if (hdr->dst_l2addr_len == 2) {
        uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);
        buf[1] |= IEEE802154_FCF_DST_ADDR_SHORT;
        buf[pos++] = dst_addr[1];
        buf[pos++] = dst_addr[0];
    }
    else if (hdr->dst_l2addr_len == 8) {
        buf[1] |= IEEE802154_FCF_DST_ADDR_LONG;
        uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);
        for (int i = 7;  i >= 0; i--) {
            buf[pos++] = dst_addr[i];
        }
    }
    else {
        /* unsupported address length */
        return 0;
    }

    /* fill in source PAN ID (if applicable */
    if (dev->options & L2_REFLECTOR_OPT_USE_SRC_PAN) {
        buf[pos++] = (uint8_t)((dev->pan) & 0xff);
        buf[pos++] = (uint8_t)((dev->pan) >> 8);
    } else {
        buf[0] |= IEEE802154_FCF_PAN_COMP;
    }

    /* fill in source address */
    if (dev->options & L2_REFLECTOR_OPT_SRC_ADDR_LONG) {
        buf[1] |= IEEE802154_FCF_SRC_ADDR_LONG;
        memcpy(&(buf[pos]), dev->addr_long, 8);
        pos += 8;
    }
    else {
        buf[1] |= IEEE802154_FCF_SRC_ADDR_SHORT;
        buf[pos++] = dev->addr_short[0];
        buf[pos++] = dev->addr_short[1];
    }

    /* set sequence number */
    buf[2] = dev->seq_nr++;
    /* return actual header length */
    return pos;
}

static gnrc_pktsnip_t *_make_netif_hdr(uint8_t *mhr)
{
    uint8_t tmp;
    uint8_t *addr;
    uint8_t src_len, dst_len;
    gnrc_pktsnip_t *snip;
    gnrc_netif_hdr_t *hdr;

    /* figure out address sizes */
    tmp = mhr[1] & IEEE802154_FCF_SRC_ADDR_MASK;
    if (tmp == IEEE802154_FCF_SRC_ADDR_SHORT) {
        src_len = 2;
    }
    else if (tmp == IEEE802154_FCF_SRC_ADDR_LONG) {
        src_len = 8;
    }
    else if (tmp == IEEE802154_FCF_SRC_ADDR_VOID) {
        src_len = 0;
    }
    else {
        return NULL;
    }
    tmp = mhr[1] & IEEE802154_FCF_DST_ADDR_MASK;
    if (tmp == IEEE802154_FCF_DST_ADDR_SHORT) {
        dst_len = 2;
    }
    else if (tmp == IEEE802154_FCF_DST_ADDR_LONG) {
        dst_len = 8;
    }
    else if (tmp == IEEE802154_FCF_DST_ADDR_VOID) {
        dst_len = 0;
    }
    else {
        return NULL;
    }
    /* allocate space for header */
    snip = gnrc_pktbuf_add(NULL, NULL, sizeof(gnrc_netif_hdr_t) + src_len + dst_len,
                           GNRC_NETTYPE_NETIF);
    if (snip == NULL) {
        return NULL;
    }
    /* fill header */
    hdr = (gnrc_netif_hdr_t *)snip->data;
    gnrc_netif_hdr_init(hdr, src_len, dst_len);
    if (dst_len > 0) {
        tmp = 5 + dst_len;
        addr = gnrc_netif_hdr_get_dst_addr(hdr);
        for (int i = 0; i < dst_len; i++) {
            addr[i] = mhr[5 + (dst_len - i) - 1];
        }
    }
    else {
        tmp = 3;
    }
    if (!(mhr[0] & IEEE802154_FCF_PAN_COMP)) {
        tmp += 2;
    }
    if (src_len > 0) {
        addr = gnrc_netif_hdr_get_src_addr(hdr);
        for (int i = 0; i < src_len; i++) {
            addr[i] = mhr[tmp + (src_len - i) - 1];
        }
    }
    return snip;
}

static int _send(gnrc_netdev_t *netdev, gnrc_pktsnip_t *pkt)
{
    l2_reflector_t *dev = (l2_reflector_t *)netdev;
    gnrc_pktsnip_t *snip, *hdr, *payload = NULL;
    gnrc_netif_hdr_t *netif;

    size_t payload_len, hdr_len;
    uint8_t mhr[IEEE802154_MAX_HDR_LEN];
    uint8_t *rcv_data;

    if (pkt == NULL) {
        return -ENOMSG;
    }
    if (dev == NULL) {
        gnrc_pktbuf_release(pkt);
        return -ENODEV;
    }

    /* abort here already if no event callback is registered */
    if (!dev->event_cb) {
        return -1;
    }

    payload_len = gnrc_pkt_len(pkt);

    /* create 802.15.4 header */
    hdr_len = _make_data_frame_hdr(dev, mhr, (gnrc_netif_hdr_t *)pkt->data);
    if (hdr_len == 0) {
        DEBUG("[l2_reflector] error: unable to create 802.15.4 header\n");
        gnrc_pktbuf_release(pkt);
        return -ENOMSG;
    }

    /* check if packet (payload) fits into max. packet size */
    snip = pkt->next;
    if ((gnrc_pkt_len(snip)) > L2_REFLECTOR_MAX_PKT_LENGTH) {
        printf("[l2_reflector] error: packet too large (%u byte) to be send\n",
               gnrc_pkt_len(snip));
        gnrc_pktbuf_release(pkt);
        return -EOVERFLOW;
    }

    /* receive part, maybe move to separate function */
    hdr = _make_netif_hdr(mhr);
    if (hdr == NULL) {
        DEBUG("[l2_reflector] error: unable to allocate netif header\n");
        return -EOVERFLOW;
    }
    /* fill missing fields in netif header */
    netif = (gnrc_netif_hdr_t *)hdr->data;
    netif->if_pid = dev->mac_pid;

    /* allocate payload */
    payload = gnrc_pktbuf_add(hdr, NULL, gnrc_pkt_len(snip), dev->proto);
    if (payload == NULL) {
        DEBUG("[l2_reflector] error: unable to allocate incoming payload\n");
        gnrc_pktbuf_release(hdr);
        gnrc_pktbuf_release(pkt);
        return -EOVERFLOW;
    }
    rcv_data = payload->data;

    /* "reverse" packet (by making it one snip) */
    while (snip != NULL) {
        memcpy(rcv_data, snip->data, snip->size);
        rcv_data += snip->size;
        snip = snip->next;
    }

    gnrc_pktbuf_release(pkt);

    DEBUG("[l2_reflector]: call event_cb function\n");

    /* finish and callback (mac layer) */
    dev->event_cb(NETDEV_EVENT_RX_COMPLETE, payload);

    /* return the number of bytes that were actually send out */
    return (int)(payload_len + hdr_len);
}

static int _add_event_cb(gnrc_netdev_t *dev, gnrc_netdev_event_cb_t cb)
{
    if (dev == NULL) {
        return -ENODEV;
    }
    if (dev->event_cb) {
        return -ENOBUFS;
    }

    dev->event_cb = cb;
    DEBUG("l2_reflector: added callback\n");
    return 0;
}

static int _rem_event_cb(gnrc_netdev_t *dev, gnrc_netdev_event_cb_t cb)
{

    if (dev == NULL) {
        return -ENODEV;
    }
    if (dev->event_cb != cb) {
        return -ENOENT;
    }

    dev->event_cb = NULL;
    DEBUG("l2_reflector: removed callback\n");
    return 0;
}

static int _get(gnrc_netdev_t *device, netopt_t opt, void *val, size_t max_len)
{
    l2_reflector_t *dev = (l2_reflector_t *) device;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case NETOPT_ADDRESS:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = l2_reflector_get_addr_short(dev);
            return sizeof(uint16_t);

        case NETOPT_ADDRESS_LONG:
            if (max_len < sizeof(uint64_t)) {
                return -EOVERFLOW;
            }
            *((uint64_t *)val) = l2_reflector_get_addr_long(dev);
            return sizeof(uint64_t);

        case NETOPT_ADDR_LEN:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = 2;
            return sizeof(uint16_t);

        case NETOPT_SRC_LEN:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            if (dev->options & L2_REFLECTOR_OPT_SRC_ADDR_LONG) {
                *((uint16_t *)val) = 8;
            }
            else {
                *((uint16_t *)val) = 2;
            }
            return sizeof(uint16_t);

        case NETOPT_NID:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = dev->pan;
            return sizeof(uint16_t);

        case NETOPT_IPV6_IID:
            if (max_len < sizeof(eui64_t)) {
                return -EOVERFLOW;
            }
            if (dev->options & L2_REFLECTOR_OPT_SRC_ADDR_LONG) {
                uint64_t addr = l2_reflector_get_addr_long(dev);
                ieee802154_get_iid(val, (uint8_t *)&addr, 8);
            }
            else {
                uint16_t addr = l2_reflector_get_addr_short(dev);
                ieee802154_get_iid(val, (uint8_t *)&addr, 2);
            }
            return sizeof(eui64_t);

        case NETOPT_PROTO:
            if (max_len < sizeof(gnrc_nettype_t)) {
                return -EOVERFLOW;
            }
            *((gnrc_nettype_t *)val) = dev->proto;
            return sizeof(gnrc_nettype_t);

        case NETOPT_MAX_PACKET_SIZE:
            if (max_len < sizeof(int16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = L2_REFLECTOR_MAX_PKT_LENGTH - _MAX_MHR_OVERHEAD;
            return sizeof(uint16_t);

        default:
            return -ENOTSUP;
    }

    return 0;
}

static int _set(gnrc_netdev_t *device, netopt_t opt, void *val, size_t len)
{
    l2_reflector_t *dev = (l2_reflector_t *) device;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {

        case NETOPT_SRC_LEN:
            if (len > sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            if (*((uint16_t *)val) == 2) {
                dev->options |= L2_REFLECTOR_OPT_SRC_ADDR_LONG;
            }
            else if (*((uint16_t *)val) == 8) {
                dev->options &= !L2_REFLECTOR_OPT_SRC_ADDR_LONG;
            }
            else {
                return -ENOTSUP;
            }
            return sizeof(uint16_t);

        case NETOPT_NID:
            if (len > sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            dev->pan = *((uint16_t *)val);
            return sizeof(uint16_t);

        default:
            return -ENOTSUP;
    }

    return 0;
}

static void _isr_event(gnrc_netdev_t *device, uint32_t event_type)
{
    (void)device;
    (void) event_type;

    DEBUG("l2_reflector: _isr_event\n");
}


const gnrc_netdev_driver_t l2_reflector_driver = {
    .send_data = _send,
    .add_event_callback = _add_event_cb,
    .rem_event_callback = _rem_event_cb,
    .get = _get,
    .set = _set,
    .isr_event = _isr_event,
};