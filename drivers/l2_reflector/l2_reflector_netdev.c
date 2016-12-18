/*
 * Copyright (C) 2016 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     l2_reflector
 * @{
 *
 * @file
 * @brief       Netdev adaption for the l2 reflector
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <string.h>
#include <assert.h>
#include <errno.h>

#include "periph/cpuid.h"
#include "net/eui64.h"
#include "net/ieee802154.h"
#include "net/netdev2.h"
#include "net/netdev2/ieee802154.h"

#include "l2_reflector.h"
#include "include/l2_reflector_netdev.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define _MAX_MHR_OVERHEAD   (25)

static int _send(netdev2_t *netdev, const struct iovec *vector, unsigned count);
static int _recv(netdev2_t *netdev, void *buf, size_t len, void *info);
static int _init(netdev2_t *netdev);
static void _isr(netdev2_t *netdev);
static int _get(netdev2_t *netdev, netopt_t opt, void *val, size_t max_len);
static int _set(netdev2_t *netdev, netopt_t opt, void *val, size_t len);

const netdev2_driver_t l2_reflector_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

void l2_reflector_setup(l2_reflector_t *dev)
{
    netdev2_t *netdev = (netdev2_t *)dev;

    netdev->driver = &l2_reflector_driver;

#if CPUID_LEN
/* make sure that the buffer is always big enough to store a 64bit value */
#   if CPUID_LEN < IEEE802154_LONG_ADDRESS_LEN
    uint8_t cpuid[IEEE802154_LONG_ADDRESS_LEN];
#   else
    uint8_t cpuid[CPUID_LEN];
#endif
    eui64_t addr_long;
#endif

#if CPUID_LEN
    /* in case CPUID_LEN < 8, fill missing bytes with zeros */
    memset(cpuid, 0, CPUID_LEN);

    cpuid_get(cpuid);

#if CPUID_LEN > IEEE802154_LONG_ADDRESS_LEN
    for (int i = IEEE802154_LONG_ADDRESS_LEN; i < CPUID_LEN; i++) {
        cpuid[i & 0x07] ^= cpuid[i];
    }
#endif
    /* make sure we mark the address as non-multicast and not globally unique */
    cpuid[0] &= ~(0x01);
    cpuid[0] |= 0x02;
    
    /* copy and set long address */
    memcpy(&addr_long, cpuid, 8);
#endif

    memcpy(dev->netdev.short_addr, &addr_long.uint16[0].u16, sizeof(uint16_t));
    memcpy(dev->netdev.long_addr, &addr_long.uint64.u64, sizeof(uint64_t));

#ifdef MODULE_GNRC_SIXLOWPAN
        dev->netdev.proto = GNRC_NETTYPE_SIXLOWPAN;
#else
        dev->netdev.proto = GNRC_NETTYPE_UNDEF;
#endif

    DEBUG("[l2_reflector]: init complete.\n");
}


static void _irq_handler(void *arg)
{
    netdev2_t *dev = (netdev2_t *) arg;

    if (dev->event_callback) {
        DEBUG("[l2_reflector]: _irq_handler\n");
        dev->event_callback(dev, NETDEV2_EVENT_ISR);
    }
}

static int _init(netdev2_t *netdev)
{
    (void)netdev;

    DEBUG("[l2_reflector]: init, nothing to do here\n");

    return 0;
}

uint8_t len_txrx = 0;
uint8_t reflect_data[IEEE802154_FRAME_LEN_MAX];

static int _send(netdev2_t *netdev, const struct iovec *vector, unsigned count)
{
    (void)netdev;
    const struct iovec *ptr = vector;

    /* reset previous value */
    len_txrx = 0;

    /* load packet data into FIFO */
    for (unsigned i = 0; i < count; i++, ptr++) {
        /* current packet data + FCS too long */
        if ((len_txrx + ptr->iov_len + 2) > L2_REFLECTOR_MAX_PKT_LENGTH) {
            DEBUG("[l2_reflector] error: packet too large (%u byte) to be send\n",
                  (unsigned)len_txrx + 2);
            return -EOVERFLOW;
        }
        /* Copy dato to send in a buffer which will be received later */
        memcpy(reflect_data+len_txrx, vector[i].iov_base, vector[i].iov_len);
        len_txrx+= vector[i].iov_len;
    }

#if ENABLE_DEBUG
    printf("[l2_reflector]: payload to send in hex:");
    for (int k=0;k<len_txrx; k++){
        printf(" 0x%x", reflect_data[k]);
    }
    printf("\n");
#endif

    DEBUG("[l2_reflector]: send packet of size %i.\n", (int)len_txrx);

    /* send data out directly if pre-loading id disabled */
    _irq_handler((void *)netdev);

    /* return the number of bytes that were actually send out */
    return (int)len_txrx;
}

static int _recv(netdev2_t *netdev, void *buf, size_t len, void *info)
{
    (void)netdev;
    (void)info;

    /* just return length when buf == NULL */
    if (buf == NULL) {
        return len_txrx;
    }
    /* not enough space in buf */
    if (len_txrx > len) {
        return -ENOBUFS;
    }

    DEBUG("[l2_reflector]: recv packet\n");
    /* copy payload */
    memcpy((uint8_t *)buf, reflect_data , len_txrx);

    return len_txrx;
}

static int _get(netdev2_t *netdev, netopt_t opt, void *val, size_t max_len)
{

    if (netdev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case NETOPT_SRC_LEN:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = NETDEV2_IEEE802154_SRC_MODE_LONG;
            return sizeof(uint16_t);
        case NETOPT_MAX_PACKET_SIZE:
            if (max_len < sizeof(int16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = L2_REFLECTOR_MAX_PKT_LENGTH - _MAX_MHR_OVERHEAD;
            return sizeof(uint16_t);

        default:
            break;
        }

    int res = netdev2_ieee802154_get((netdev2_ieee802154_t *)netdev, opt, val,
                                       max_len);

    return res;
}

static int _set(netdev2_t *netdev, netopt_t opt, void *val, size_t len)
{
    l2_reflector_t *dev = (l2_reflector_t *) netdev;
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }
    res = netdev2_ieee802154_set((netdev2_ieee802154_t *)netdev, opt,
                                     val, len);
    return res;
}

static void _isr(netdev2_t *netdev)
{

    netdev->event_callback(netdev, NETDEV2_EVENT_RX_COMPLETE);

}
