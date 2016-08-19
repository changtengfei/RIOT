/*
 * Copyright (C) 2016 Alexander Aring <aar@pengutronix.de>
 *                    Freie Universität Berlin
 *                    HAW Hamburg
 *                    Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_sock_tcp    TCP sock API
 * @ingroup     net_sock
 * @brief       Sock submodule for TCP
 * @{
 *
 * @file
 * @brief   TCP sock definitions
 *
 * @author  Alexander Aring <aar@pengutronix.de>
 * @author  Simon Brummer <simon.brummer@haw-hamburg.de>
 * @author  Cenk Gündoğan <mail@cgundogan.de>
 * @author  Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @author  Martine Lenders <m.lenders@fu-berlin.de>
 * @author  Kaspar Schleiser <kaspar@schleiser.de>
 */
#ifndef NET_SOCK_TCP_H_
#define NET_SOCK_TCP_H_

#include <stdint.h>
#include <stdlib.h>

#include "net/sock/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   An end point for a TCP sock object
 */
typedef struct {
    sock_addr_ip_t addr;    /**< IP address */
    int family;             /**< family of sock_tcp_ep_t::addr */

    /**
     * @brief   network interface ID as defined in @ref net_netif
     *
     * Use @ref SOCK_ADDR_ANY_NETIF for any interface.
     * For reception this is the local interface the message came over,
     * for transmission, this is the local interface the message should be send
     * over
     */
    uint16_t netif;
    uint16_t port;          /**< port for the TCP end point */
} sock_tcp_ep_t;

/**
 * @brief   Implementation-specific type of a TCP sock object
 *
 * `struct sock_tcp` needs to be defined by stack-specific implementation.
 */
typedef struct sock_tcp sock_tcp_t;

/**
 * @brief   Implementation-specific type of a TCP listening queue
 *
 * `struct sock_tcp_queue` needs to be defined by stack-specific implementation.
 */
typedef struct sock_tcp_queue sock_tcp_queue_t;

/**
 * @brief   Establishes a new TCP sock connection
 *
 * @pre `sock != NULL`
 * @pre `remote != NULL`
 * @pre `local_port != 0`
 *
 * @param[out] sock     Preallocated sock object. Must fill the size of
 *                      the stack-specific sock descriptor.
 * @param[in] remote    Remote end point for the sock object.
 * @param[in] flags     Flags for the sock object. See also @ref net_sock_flags.
 *                      May be 0.
 *
 * @return  0 on success.
 * @return  -EADDRINUSE, if `(flags & SOCK_FLAGS_REUSE_EP) == 0` and
 *          @p local_port is already used elsewhere
 * @return  -EAFNOSUPPORT, if sock_tcp_ep_t::family of @p remote is not
 *          supported.
 * @return  -ECONNREFUSED, if no-one is listening on the @p remote end point.
 * @return  -EINVAL, if sock_tcp_ep_t::netif of @p remote is not a valid
 *          interface.
 * @return  -ENETUNREACH, if network defined by @p remote is not reachable.
 * @return  -ENOMEM, if system was not able to allocate sufficient memory to
 *          establish connection.
 * @return  -EPERM, if connections to @p remote are not permitted on the system
 *          (e.g. by firewall rules).
 * @return  -ETIMEDOUT, if the connection attempt to @p remote timed out.
 */
int sock_tcp_connect(sock_tcp_t *sock, const sock_tcp_ep_t *remote,
                     uint16_t local_port, uint32_t flags);

/**
 * @brief   Listen for an incoming connection request on @p local end point
 *
 * @param[in] queue         Preallocated listening queue object. Must fill the
 *                          size of the stack-specific queue descriptor.
 * @param[in] remote        Local end point to listen on.
 * @param[in] queue_array   Preallocated array of sock objects. Each sock object
 *                          must fill the size of the stack-specific sock
 *                          descriptor.
 * @param[in] queue_len     Length of @p queue_array.
 * @param[in] flags         Flags for the listening queue. See also
 *                          @ref net_sock_flags. May be 0.
 *
 * @return  0 on success.
 * @return  -EADDRINUSE, if `(flags & SOCK_FLAGS_REUSE_EP) == 0` and
 *          @p local_port is already used elsewhere
 * @return  -EAFNOSUPPORT, if sock_tcp_ep_t::family of @p local is not
 *          supported.
 * @return  -EINVAL, if sock_tcp_ep_t::netif of @p local is not a valid
 *          interface.
 */
int sock_tcp_listen(sock_tcp_queue_t *queue, const sock_tcp_ep_t *local,
                    sock_tcp_t[] queue_array, unsigned queue_len,
                    uint32_t flags);

/**
 * @brief   Disconnects a TCP connection
 *
 * @pre `(sock != NULL)`
 *
 * @param[in] sock  A TCP sock object.
 */
void sock_tcp_disconnect(sock_tcp_t *sock);

/**
 * @brief   Stops listening on TCP listening queue
 *
 * @param[in] queue A TCP listening queue.
 */
void sock_tcp_stop_listen(sock_tcp_queue_t *queue);

/**
 * @brief   Gets the local end point of a TCP sock object
 *
 * @pre `(sock != NULL) && (ep != NULL)`
 *
 * @param[in] sock  A TCP sock object.
 * @param[out] ep   The local end point.
 *
 * @return  0 on success.
 * @return  -EADDRNOTAVAIL, when @p sock has no local end point.
 */
int sock_tcp_get_local(sock_tcp_t *sock, sock_tcp_ep_t *ep);

/**
 * @brief   Gets the remote end point of a TCP sock object
 *
 * @pre `(sock != NULL) && (ep != NULL)`
 *
 * @param[in] sock  A TCP sock object.
 * @param[out] ep   The remote end point.
 *
 * @return  0 on success.
 * @return  -ENOTCONN, when @p sock is not connected to a remote end point.
 */
int sock_tcp_get_remote(sock_tcp_t *sock, sock_tcp_ep_t *ep);

/**
 * @brief   Receives and handles TCP connection requests from other peers
 *
 * @pre `(queue != NULL) && (sock != NULL)`
 *
 * @param[in] sock      A TCP listening queue.
 * @param[out] out_sock A new TCP sock object for the established
 *                      sock object.
 *
 * @return  0 on success.
 * @return  -ENOMEM, if system was not able to allocate sufficient memory to
 *          establish connection.
 * @return  -EPERM, if connections on local end point of @p queue are not
 *          permitted on this system (e.g. by firewall rules).
 * @return  -ETIMEDOUT, if the operation timed out stack-internally.
 */
int sock_tcp_accept(sock_tcp_queue_t *queue, sock_tcp_t **sock);

/**
 * @brief   Reads data from an established TCP stream
 *
 * @pre `(sock != NULL) && (data != NULL) && (max_len > 0)`
 *
 * @param[in] sock      A TCP sock object.
 * @param[out] data     Pointer where the read data should be stored.
 * @param[in] max_len   Maximum space available at @p data.
 *                      If read data exceeds @p max_len the data is
 *                      truncated and the remaining data can be retrieved
 *                      later on.
 * @param[in] timeout   Timeout for receive in microseconds.
 *                      This value can be ignored (no timeout) if the
 *                      @ref sys_xtimer module is not present and the stack does
 *                      not support timeouts on its own.
 *                      May be 0 for no timeout.
 *
 * @note    Function may block.
 *
 * @return  The number of bytes read on success.
 * @return  0, if no read data is available, but everything is in order.
 * @return  -EADDRNOTAVAIL, if local of @p sock is not given.
 * @return  -ECONNREFUSED, if remote end point of @p sock refused to allow the
 *          connection.
 * @return  -ENOTCONN, when @p sock is not connected to a remote end point.
 * @return  -ETIMEDOUT, if @p timeout expired.
 */
int sock_tcp_read(sock_tcp_t *sock, void *data, size_t max_len,
                  uint32_t timeout);

/**
 * @brief   Writes data to an established TCP stream
 *
 * @pre `(sock != NULL) && (data != NULL) && (max > 0)`
 *
 * @param[in] sock  A TCP sock object.
 * @param[in] data  Pointer to the data to be written to the stream.
 * @param[in] len   Maximum space available at @p data.
 *
 * @note    Function may block.
 *
 * @return  The number of bytes written on success.
 * @return  -ECONNRESET, if connection was reset by remote end point.
 * @return  -ENOMEM, if no memory was available to written @p data.
 * @return  -ENOTCONN, if @p sock is not connected to a remote end point.
 */
int sock_tcp_write(sock_tcp_t *sock, const void *data, size_t len);

#ifdef MODULE_EMP6_SOCK_TCP
#include "emb6/sock.h"
#endif

#ifdef MODULE_GNRC_SOCK_TCP
#include "net/gnrc/sock.h"
#endif

#ifdef MODULE_LWIP_SOCK_TCP
#include "lwip/sock.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* NET_SOCK_TCP_H_ */
/** @} */
