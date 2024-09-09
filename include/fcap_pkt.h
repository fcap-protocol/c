#ifndef FCAP_PKT_H
#define FCAP_PKT_H

#include "fcap_bp.h"

typedef uint8_t *bytes_t;
typedef uint16_t node_id_t;
typedef uint16_t mid_t;
typedef uint16_t pkt_len_t;
// typedef bytes_t body_t;

typedef enum fcap_error {
	FCAP_OK = 0,
	FCAP_EINDEX = 1,
	FCAP_ENOMEM = 2,
	FCAP_EINVAL = 3,
	FCAP_EABORT = 4
	// FCAP_EEXIST = 2,
	// FCAP_ENOKEY = 4,
	// FCAP_ETYPE = 5,
} FError;

struct fcap_endpoint;
typedef struct fcap_endpoint *FEndpoint;

struct pkt_buffer {
	bytes_t buf_start;
	bytes_t buf_cur;
	bytes_t buf_end;
};
typedef struct pkt_buffer *FPktBuffer;

struct fcap_request {
	FEndpoint endpoint;
	bool is_inbound;
	uint16_t cmd;
	struct {
		bool sent;
		mid_t id;
		struct pkt_buffer buf;
	} _priv;
};
typedef struct fcap_request *FRequest;

struct fcap_response {
	FEndpoint endpoint;
	bool is_inbound;
	Status status;
	struct {
		bool sent;
		mid_t id;
		struct pkt_buffer buf;
	} _priv;
};
typedef struct fcap_response *FResponse;

struct fcap_pkt {
	bool is_response;
	FRequest req;
	FResponse res;
};
typedef struct fcap_pkt *FPkt;

void fcap_request_init(FRequest req, FEndpoint endpoint, bytes_t buf, pkt_len_t cur_len, pkt_len_t total_len);
void fcap_response_init(FResponse res, FEndpoint endpoint, bytes_t buf, pkt_len_t cur_len, pkt_len_t total_len);

pkt_len_t fcap_request_append(FRequest req, bytes_t playload, pkt_len_t len);
pkt_len_t fcap_response_append(FResponse res, bytes_t playload, pkt_len_t len);

FError fcap_pkt_encode(FPkt pkt, bytes_t buf, pkt_len_t *cur_len, pkt_len_t total_len);
FError fcap_pkt_decode(FPkt pkt, FEndpoint endpoint, bytes_t buf, pkt_len_t cur_len, pkt_len_t total_len);

#endif
