#ifndef FCAP_H
#define FCAP_H

#include "fcap_bp.h"
#include "fcap_pkt.h"

// #define MAX_ENDPOINTS 2

struct fcap;
typedef struct fcap *FApp;

struct fcap_address {
	uint8_t data[8];
};
typedef struct fcap_address *FAddress;

struct fcap_transport {
	const void *priv;
	int (*receive_bytes)(const void *priv, FAddress addr, bytes_t bytes, size_t length);
	int (*send_bytes)(const void *priv, FAddress addr, bytes_t bytes, size_t length);
};
typedef struct fcap_transport *FTransport;

struct fcap_endpoint {
	FTransport transport;
	struct fcap_address address;
	node_id_t node_id;
};
typedef struct fcap_endpoint *FEndpoint;

typedef FError (*NextReq)(FApp app, FRequest req, FResponse res);
typedef FError (*NextRes)(FApp app, FResponse res);

struct fcap_middleware {
	const void *priv;
	FError (*on_request)(const void *priv, FApp app, FRequest req, FResponse res, NextReq next);
	FError (*on_response)(const void *priv, FApp app, FResponse res, NextRes next);
};
typedef struct fcap_middleware *FMiddleware;

struct fcap {
	const uint8_t num_transports;
	const uint8_t num_middleware;
	const FTransport *transports;
	const FMiddleware *middleware;
	// FEndPoint endpoints[MAX_ENDPOINTS];
	struct {
		bool middleware_dir;
		uint8_t middleware_i;
		uint8_t in_buf[MTU];
		uint8_t out_buf[MTU];
	} _priv;
};
typedef struct fcap *FApp;

#define FCAP_CREATE_TRANSPORT(name, type, ptr, rec_fn, send_fn)                                                        \
	struct type name##_priv;                                                                                       \
	const struct fcap_transport t_##name = {                                                                       \
		.priv = &name##_priv,                                                                                  \
		.receive_bytes = rec_fn,                                                                               \
		.send_bytes = send_fn,                                                                                 \
	};                                                                                                             \
	const ptr name = (ptr) & name##_priv;

#define FCAP_SET_TRANSPORTS(name, ...)                                                                                 \
	const FTransport name[] = { __VA_ARGS__ };                                                                     \
	const uint8_t name##_size = sizeof(name) / sizeof(FTransport);

#define FCAP_SET_MIDDLEWARE(name, ...)                                                                                 \
	const FMiddleware name[] = { __VA_ARGS__ };                                                                    \
	const uint8_t name##_size = sizeof(name) / sizeof(FMiddleware);

#define FCAP_CREATE_APP(name, transports, middleware)                                                                  \
	struct fcap name##_internal = {                                                                                \
		.num_transports = transports##_size,                                                                   \
		.num_middleware = middleware##_size,                                                                   \
		.transports = transports,                                                                              \
		.middleware = middleware,                                                                              \
	};                                                                                                             \
	const FApp name = &name##_internal;

void fcap_init(FApp app);

FError fcap_poll(FApp app);

// FError fcap_send_req(FApp app, FRequest req);

FError fcap_send_response(FApp app, FRequest req, FResponse res);

#endif