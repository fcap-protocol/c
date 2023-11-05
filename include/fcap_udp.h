#ifndef FCAP_UDP_H
#define FCAP_UDP_H

#include <netinet/ip.h>

typedef struct fcap_udp {
	int sockfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in dest_addr;
} fcap_udp_t;

/**
 * @brief send bytes function as per fcap.h spec
*/
int fcap_udp_send_bytes(void *priv, uint8_t *bytes, size_t length);

// /**
//  * @brief poll function as per fcap.h spec
// */
// int fcap_udp_poll(void *priv);

/**
 * @brief get bytes function as per fcap.h spec
*/
int fcap_udp_get_bytes(void *priv, uint8_t *bytes, size_t length);

#define FCAP_CREATE_UDP_TRANSPORT(name)                                        \
	struct fcap_udp name##_priv;                                           \
	struct fcap_transport name = {                                         \
		.priv = &name##_priv,                                          \
		.get_bytes = fcap_udp_get_bytes,                               \
		.send_bytes = fcap_udp_send_bytes,                             \
	};

/**
 * @brief Sets up a udp socket which binds to any ip address on the host
 * on the specifed server port
 * @param udp the udp transport struct
 * @param server_port the port to listen to
 * @param dest_ip the ip address of the peer
 * @param dest_port the port of the peer
 * @returns 0 on success or -errno on failure
*/
int fcap_udp_setup_transport(void *priv,
			     int server_port,
			     char *dest_ip,
			     int dest_port);

/**
 * @brief closes the socket, should be called on shutdown
 * @param priv the udp transport struct
*/
void fcap_udp_cleanup(void *priv);

#endif /* FCAP_UDP_H */
