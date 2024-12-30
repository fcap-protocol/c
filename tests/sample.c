#include <fcap.h>
#include <transport_udp.h>

#define BYTES_TO_IP_INT(x) (*(uint32_t *)(uint8_t *)x)

FCAP_CREATE_UDP_TRANSPORT(udp);

FCAP_SET_TRANSPORTS(transports, &t_udp);

FError on_req(const void *priv, FApp app, FRequest req, FResponse res, NextReq next)
{
	if (!req->is_inbound)
		return next(app, req, res);

	res->status = 255;
	uint8_t buf[] = { 127 };
	fcap_response_payload_append(res, buf, sizeof(buf));
	fcap_send_response(app, req, res);
	return FCAP_OK;
}

FError on_res(const void *priv, FApp app, FResponse res, NextRes next)
{
		return next(app, res);
}

struct fcap_middleware handler = { .priv = (void *)1, .on_request = on_req, .on_response = on_res };

FCAP_SET_MIDDLEWARE(middleware, &handler);

FCAP_CREATE_APP(app, transports, middleware);

int main()
{
	fcap_init(app);
	transport_udp_init(udp, 1434);

	struct fcap_endpoint server;
	uint8_t ip[4] = { 127, 0, 0, 1 };
	transport_udp_endpoint_bind(&t_udp, &server, BYTES_TO_IP_INT(ip), 1434);

	struct fcap_request req;
	fcap_request_bind(app, &req, &server);
	req.cmd = 101;
	fcap_request_payload_append(&req, (bytes_t)(&(uint8_t[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), 10);

	fcap_send_request(app, &req);

	while (true) {
		fcap_poll(app);
	}
}
