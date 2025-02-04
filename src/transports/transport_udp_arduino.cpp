#ifdef ARDUINO
extern "C" {
#include <fcap.h>
}
#include <transports/transport_udp_arduino.hpp>

int transport_udp_init(FTransportUdp udp, uint16_t server_port)
{
	udp->client = WiFiUDP();
	udp->client.begin(server_port);
	return 0;
}

void transport_udp_deinit(FTransportUdp udp)
{
	udp->client.stop();
}

void transport_udp_endpoint_bind(FTransport transport, FEndpoint endpoint, uint32_t addr, uint16_t port)
{
	fcap_endpoint_init(endpoint, transport);
	memcpy(&endpoint->address.data[0], &addr, sizeof(addr));
	memcpy(&endpoint->address.data[sizeof(addr)], &port, sizeof(port));
}

int transport_udp_receive_bytes(const void *priv, FAddress addr, bytes_t bytes, size_t length)
{
	FTransportUdp udp = (FTransportUdp)priv;

	int ret = udp->client.parsePacket();
	if (ret == 0)
		return FCAP_OK;
	uint32_t ip = udp->client.remoteIP();
	uint16_t port = udp->client.remotePort();
	udp->client.read(bytes, length);

	memcpy(&addr->data[0], &ip, sizeof(ip));
	memcpy(&addr->data[sizeof(ip)], &port, sizeof(port));
	return ret;
}

int transport_udp_send_bytes(const void *priv, FAddress addr, bytes_t bytes, size_t length)
{
	FTransportUdp udp = (FTransportUdp)priv;

	uint32_t ip;
	uint16_t port;
	memcpy(&ip, &addr->data[0], sizeof(ip));
	memcpy(&port, &addr->data[sizeof(ip)], sizeof(port));

	FCAP_DEBUG("Host: %u.%u.%u.%u:%u\n", addr->data[0], addr->data[1], addr->data[2], addr->data[3], port);

	udp->client.beginPacket(ip, port);
	udp->client.write(bytes, length);
	bool sent = udp->client.endPacket();

	if (!sent)
		return FCAP_EINVAL;

	return length;
}
#endif