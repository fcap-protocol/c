#include <fcap.h>
#include <fcap_udp.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

int fcap_udp_send_bytes(void *priv, uint8_t *bytes, size_t length)
{
	int ret;
	fcap_udp_t *udp = priv;

	ret = sendto(udp->sockfd,
		     (const char *)bytes,
		     length,
		     0,
		     (const struct sockaddr *)&udp->dest_addr,
		     sizeof(udp->dest_addr));

	if (ret != length)
		return -FCAP_EINVAL;

	return ret;
}

int fcap_udp_poll(void *priv)
{
	int ret;
	fcap_udp_t *udp = priv;

	struct pollfd poll_fds[1];
	poll_fds[0].fd = udp->sockfd;
	poll_fds[0].events = POLLIN | POLLPRI;

	/* We poll a single fd to check if it's ready*/
	ret = poll(poll_fds, 1, 0);

	if (ret < 0) {
		return -FCAP_EINVAL;
	}

	return ret;
}

int fcap_udp_get_bytes(void *priv, uint8_t *bytes, size_t length)
{
	fcap_udp_t *udp = priv;
	return recv(udp->sockfd, bytes, length, MSG_DONTWAIT);
}

int fcap_udp_setup_channel(void *priv,
			   int server_port,
			   char *dest_ip,
			   int dest_port)
{
	int ret;
	fcap_udp_t *udp = priv;

	/* Creating socket file descriptor */
	if ((udp->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return udp->sockfd;

	memset(&udp->server_addr, 0, sizeof(udp->server_addr));

	/* 
	 * Filling server information 
	 * Set IPv4, listen on `server_port` and listen on any host ip
	 */
	udp->server_addr.sin_family = AF_INET;
	udp->server_addr.sin_addr.s_addr = INADDR_ANY;
	udp->server_addr.sin_port = htons(server_port);

	/* Bind the socket with the server address */
	ret = bind(udp->sockfd,
		   (const struct sockaddr *)&udp->server_addr,
		   sizeof(udp->server_addr));
	if (ret < 0) {
		return ret;
	}

	/*
	 * Filling in client information
	 */
	memset(&udp->dest_addr, 0, sizeof(udp->dest_addr));
	udp->dest_addr.sin_family = AF_INET;
	udp->dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
	udp->dest_addr.sin_port = htons(dest_port);
}

void fcap_udp_cleanup(void *priv)
{
	fcap_udp_t *udp = priv;
	close(udp->sockfd);
}
