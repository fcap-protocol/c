#ifdef __unix__
#ifndef TRANSPORT_UDP_H
#define TRANSPORT_UDP_H

#include <fcap.h>

#include <netinet/ip.h>

struct transport_udp {
	int sockfd;
	struct sockaddr_in server_addr;
};
typedef struct transport_udp *FTransportUdp;

/**
 * @brief Sets up a udp socket which binds to any ip address on the host
 * on the specifed server port
 * @param udp the udp transport struct
 * @param server_port the port to listen to
 * @returns 0 on success or -errno on failure
*/
int transport_udp_init(FTransportUdp udp, uint16_t server_port);

/**
 * @brief closes the socket, should be called on shutdown
 * @param udp the udp transport struct
*/
void transport_udp_deinit(FTransportUdp udp);

void transport_udp_endpoint_bind(FTransport transport, FEndpoint endpoint, in_addr_t addr, uint16_t port);

/**
 * @brief get bytes function as per fcap.h spec
*/
int transport_udp_receive_bytes(const void *priv, FAddress addr, bytes_t bytes, size_t length);

/**
 * @brief send bytes function as per fcap.h spec
*/
int transport_udp_send_bytes(const void *priv, FAddress addr, bytes_t bytes, size_t length);

#define FCAP_CREATE_UDP_TRANSPORT(name)                                                                                \
	FCAP_CREATE_TRANSPORT(name, transport_udp, FTransportUdp, transport_udp_receive_bytes, transport_udp_send_bytes)

#endif
#endif