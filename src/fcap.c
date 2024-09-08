
#include "stdio.h"
#include "string.h"
#include "fcap.h"

static FError fcap_do_req_middleware(FApp app, FRequest req, FResponse res);
static FError fcap_do_res_middleware(FApp app, FResponse res);

void fcap_init(FApp app)
{
	// memset(app->_priv.endpoints, 0, sizeof(app->endpoints));
	memset(app->_priv.in_buf, 0, sizeof(app->_priv.in_buf));
	memset(app->_priv.out_buf, 0, sizeof(app->_priv.out_buf));
}

void fcap_endpoint_init(FEndpoint endpoint, FTransport transport)
{
	memset(endpoint, 0, sizeof(struct fcap_endpoint));
	endpoint->transport = transport;
}

FError fcap_poll(FApp app)
{
	FError err;

	bytes_t buf = app->_priv.in_buf;
	const pkt_len_t buf_len = sizeof(app->_priv.in_buf);

	struct fcap_endpoint endpoint;
	FTransport transport;

	struct fcap_request req;
	struct fcap_response res;
	struct fcap_pkt pkt = { .is_response = false, .req = &req, .res = &res };

	for (uint8_t i = 0; i < app->num_transports; i++) {
		transport = app->transports[i];
		fcap_endpoint_init(&endpoint, transport);

		int ret = transport->receive_bytes(transport->priv, &endpoint.address, buf, buf_len);
		// Error
		if (ret < 0)
			return ret;
		// No Data
		if (ret == 0)
			continue;

		// Parse Packer
		err = fcap_pkt_decode(&pkt, &endpoint, buf, ret, buf_len);
		if (err)
			return err;

		if (!pkt.is_response) { // if req
			// prepare response
			fcap_response_init(pkt.res, &endpoint, app->_priv.out_buf, 0, sizeof(app->_priv.out_buf));
			pkt.res->_priv.id = pkt.req->_priv.id;
			pkt.res->_priv.sent = false;

			// run middleware
			err = fcap_do_req_middleware(app, pkt.req, pkt.res);
			if (err && err != FCAP_EINDEX)
				return err;
		} else {
			// run middleware
			err = fcap_do_res_middleware(app, pkt.res);
			if (err && err != FCAP_EINDEX)
				return err;
		}
	}
	return FCAP_OK;
}

// FError fcap_send_req(FApp app, FRequest req)
// {
// }

FError fcap_send_response(FApp app, FRequest req, FResponse res)
{
	FError err;

	// run middleware
	err = fcap_do_res_middleware(app, res);
	if (err && err != FCAP_EINDEX)
		return err;

	// encode
	struct fcap_pkt pkt = { .is_response = true, .req = NULL, .res = res };
	bytes_t buff = app->_priv.out_buf;
	pkt_len_t cur_len = 0;

	err = fcap_pkt_encode(&pkt, buff, &cur_len, sizeof(app->_priv.out_buf));
	if (err)
		return err;

	// send to transport
	FTransport transport = res->endpoint->transport;
	if (transport == NULL)
		return FCAP_EINVAL;
	int ret = transport->send_bytes(transport->priv, &res->endpoint->address, buff, cur_len);
	if (ret < 0)
		return ret;
	// No Data
	if (ret == 0)
		return FCAP_EINVAL;

	return FCAP_OK;
}

// Private

static FError fcap_next_req_middleware(FApp app, FRequest req, FResponse res)
{
	if (app->_priv.middleware_i >= app->num_middleware)
		return FCAP_EINDEX;
	FMiddleware middleware = app->middleware[app->_priv.middleware_i];
	app->_priv.middleware_i += app->_priv.middleware_dir ? 1 : -1;
	if (middleware->on_request)
		return middleware->on_request(middleware->priv, app, req, res, fcap_next_req_middleware);
	else
		return fcap_next_req_middleware(app, req, res);
}

static FError fcap_do_req_middleware(FApp app, FRequest req, FResponse res)
{
	if (!app->num_middleware)
		return FCAP_OK;
	app->_priv.middleware_dir = !req->is_inbound;
	app->_priv.middleware_i = req->is_inbound ? app->num_middleware - 1 : 0;
	return fcap_next_req_middleware(app, req, res);
}

static FError fcap_next_res_middleware(FApp app, FResponse res)
{
	if (app->_priv.middleware_i >= app->num_middleware)
		return FCAP_EINDEX;
	FMiddleware middleware = app->middleware[app->_priv.middleware_i];
	app->_priv.middleware_i += app->_priv.middleware_dir ? 1 : -1;
	if (middleware->on_response)
		return middleware->on_response(middleware->priv, app, res, fcap_next_res_middleware);
	else
		return fcap_next_res_middleware(app, res);
}

static FError fcap_do_res_middleware(FApp app, FResponse res)
{
	if (!app->num_middleware)
		return FCAP_OK;
	app->_priv.middleware_dir = !res->is_inbound;
	app->_priv.middleware_i = res->is_inbound ? app->num_middleware - 1 : 0;
	return fcap_next_res_middleware(app, res);
}
