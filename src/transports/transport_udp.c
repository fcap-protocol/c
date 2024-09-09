#ifdef linux

#include <transports/transport_udp.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int transport_udp_receive_bytes(const void *priv, FAddress addr, bytes_t bytes, size_t length)
{
	FTransportUdp udp = (FTransportUdp)priv;

	struct sockaddr_in soc_addr;
	socklen_t soc_addr_len = sizeof(soc_addr);
	int ret = recvfrom(udp->sockfd, bytes, length, MSG_DONTWAIT, (struct sockaddr *)&soc_addr, &soc_addr_len);

	/* Normalize errors */
	if (ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return FCAP_OK;
		else
			return FCAP_EINVAL;
	}
	// Copy addr + port = 6 bytes
	memcpy(&addr->data[0], &soc_addr.sin_addr, sizeof(soc_addr.sin_addr));
	memcpy(&addr->data[sizeof(soc_addr.sin_addr)], &soc_addr.sin_port, sizeof(soc_addr.sin_port));
	return ret;
}

int transport_udp_send_bytes(const void *priv, FAddress addr, bytes_t bytes, size_t length)
{
	FTransportUdp udp = (FTransportUdp)priv;

	// Copy addr + port = 6 bytes
	struct sockaddr_in soc_addr;
	soc_addr.sin_family = AF_INET;
	memcpy(&soc_addr.sin_addr, &addr->data[0], sizeof(soc_addr.sin_addr));
	memcpy(&soc_addr.sin_port, &addr->data[sizeof(soc_addr.sin_addr)], sizeof(soc_addr.sin_port));

	int ret = sendto(
		udp->sockfd, (const bytes_t)bytes, length, 0, (const struct sockaddr *)&soc_addr, sizeof(soc_addr));

	if (ret != length)
		return FCAP_EINVAL;

	return ret;
}

int transport_udp_init(const void *priv, uint16_t server_port)
{
	FTransportUdp udp = (FTransportUdp)priv;
	int ret;

	memset(udp, 0, sizeof(struct transport_udp));

	/* Creating socket file descriptor */
	if ((udp->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return udp->sockfd;

	/* 
	 * Filling server information 
	 * Set IPv4, listen on `server_port` and listen on any host ip
	 */
	udp->server_addr.sin_family = AF_INET;
	udp->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	udp->server_addr.sin_port = htons(server_port);

	/* Bind the socket with the server address */
	ret = bind(udp->sockfd, (const struct sockaddr *)&udp->server_addr, sizeof(udp->server_addr));
	if (ret < 0) {
		return ret;
	}

	/*
	 * Filling in client information
	 */
	// memset(&udp->dest_addr, 0, sizeof(udp->dest_addr));
	// udp->dest_addr.sin_family = AF_INET;
	// udp->dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
	// udp->dest_addr.sin_port = htons(dest_port);

	return 0;
}

void transport_udp_deinit(const void *priv)
{
	FTransportUdp udp = (FTransportUdp)priv;

	close(udp->sockfd);
	udp->sockfd = -1;
}

#endif
