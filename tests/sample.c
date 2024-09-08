#include <fcap.h>
#include <transports/transport_udp.h>

FCAP_CREATE_UDP_TRANSPORT(udp);

FCAP_SET_TRANSPORTS(transports, (FTransport)&t_udp);

FError on_req(const void *priv, FApp app, FRequest req, FResponse res, NextReq next)
{
	if (req->is_inbound) {
		res->status = 255;
		uint8_t buf[] = { 128 };
		fcap_response_append(res, buf, sizeof(buf));
		fcap_send_response(app, req, res);
		return FCAP_OK;
	} else {
		return next(app, req, res);
	}
}

struct fcap_middleware handler = { .priv = (void *)1, .on_request = on_req, .on_response = NULL };

FCAP_SET_MIDDLEWARE(middleware, &handler);

FCAP_CREATE_APP(app, transports, middleware);

int main()
{
	fcap_init(app);
	transport_udp_init(udp, 1434);

	while (true) {
		fcap_poll(app);
	}
}
