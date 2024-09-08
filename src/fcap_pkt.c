#include "string.h"
#include "fcap_pkt.h"

static pkt_len_t fcap_pkt_buffer_remaining(FPktBuffer pkt_buf)
{
	return pkt_buf->buf_cur - pkt_buf->buf_start;
}

static pkt_len_t fcap_pkt_buffer_used(FPktBuffer pkt_buf)
{
	return pkt_buf->buf_end - pkt_buf->buf_cur;
}

bytes_t fcap_pkt_buffer_alloc(FPktBuffer pkt_buf, pkt_len_t len)
{
	pkt_len_t remaining = fcap_pkt_buffer_remaining(pkt_buf);
	if (remaining < len)
		return NULL;
	pkt_buf->buf_cur -= len;
	return pkt_buf->buf_cur;
}

pkt_len_t fcap_pkt_buffer_free(FPktBuffer pkt_buf, pkt_len_t len)
{
	pkt_len_t used = fcap_pkt_buffer_used(pkt_buf);
	if (used < len)
		return 0;
	pkt_buf->buf_cur += len;
	return len;
}

pkt_len_t fcap_pkt_buffer_append(FPktBuffer pkt_buf, bytes_t buf, pkt_len_t len)
{
	bytes_t pkt_cur = fcap_pkt_buffer_alloc(pkt_buf, len);
	if (!pkt_cur)
		return 0;
	memcpy(pkt_cur, buf, len);
	return len;
}

static bool fcap_pkt_buffer_init(FPktBuffer pkt_buf, bytes_t buf, pkt_len_t cur_len, pkt_len_t total_len)
{
	if (total_len == 0 || total_len > MTU)
		return false;
	pkt_buf->buf_start = buf;
	pkt_buf->buf_end = buf + total_len;
	pkt_buf->buf_cur = pkt_buf->buf_end;

	if (cur_len > 0) {
		bytes_t pkt_cur = fcap_pkt_buffer_alloc(pkt_buf, cur_len);
		memmove(pkt_cur, buf, cur_len);
	}
	return true;
}

static void fcap_pkt_buffer_deinit(FPktBuffer pkt_buf, bytes_t buf, pkt_len_t *cur_len, pkt_len_t total_len)
{
	*cur_len = fcap_pkt_buffer_used(pkt_buf);
	memmove(buf, pkt_buf->buf_cur, *cur_len);
	memset(pkt_buf, 0, sizeof(struct pkt_buffer));
}

void fcap_request_init(FRequest req, FEndpoint endpoint, bytes_t buf, pkt_len_t cur_len, pkt_len_t total_len)
{
	memset(req, 0, sizeof(struct fcap_request));
	req->endpoint = endpoint;
	fcap_pkt_buffer_init(&req->_priv.buf, buf, cur_len, total_len);
}

void fcap_response_init(FResponse res, FEndpoint endpoint, bytes_t buf, pkt_len_t cur_len, pkt_len_t total_len)
{
	memset(res, 0, sizeof(struct fcap_response));
	res->endpoint = endpoint;
	fcap_pkt_buffer_init(&res->_priv.buf, buf, cur_len, total_len);
}

pkt_len_t fcap_request_append(FRequest req, bytes_t playload, pkt_len_t len)
{
	return fcap_pkt_buffer_append(&req->_priv.buf, playload, len);
}

pkt_len_t fcap_response_append(FResponse res, bytes_t playload, pkt_len_t len)
{
	return fcap_pkt_buffer_append(&res->_priv.buf, playload, len);
}

FError fcap_pkt_encode(FPkt pkt, bytes_t buf, pkt_len_t *cur_len, pkt_len_t total_len)
{
	if (!pkt->is_response) {
		pkt_len_t len = fcap_pkt_buffer_used(&pkt->req->_priv.buf);
		uint8_t header_buf[HEADER_LEN] = {};
		struct PacketRequest pkt_req = {
			.header = { .version = VERSION_V1,
				    .is_res = false,
				    .reserved = 0,
				    .id = pkt->req->_priv.id,
				    .len = len },
			.cmd = pkt->req->cmd,
		};
		EncodePacketRequest(&pkt_req, (bytes_t)&header_buf);
		if (!fcap_pkt_buffer_append(&pkt->req->_priv.buf, header_buf, sizeof(header_buf)))
			return FCAP_ENOMEM;
		fcap_pkt_buffer_deinit(&pkt->req->_priv.buf, buf, cur_len, total_len);
	} else {
		pkt_len_t len = fcap_pkt_buffer_used(&pkt->res->_priv.buf);
		uint8_t header_buf[HEADER_LEN] = {};
		struct PacketResponse pkt_res = {
			.header = { .version = VERSION_V1,
				    .is_res = false,
				    .reserved = 0,
				    .id = pkt->res->_priv.id,
				    .len = len },
			.status = pkt->res->status,
		};
		EncodePacketResponse(&pkt_res, (bytes_t)&header_buf);
		if (!fcap_pkt_buffer_append(&pkt->res->_priv.buf, header_buf, sizeof(header_buf)))
			return FCAP_ENOMEM;
		fcap_pkt_buffer_deinit(&pkt->res->_priv.buf, buf, cur_len, total_len);
	}

	return FCAP_OK;
}

FError fcap_pkt_decode(FPkt pkt, FEndpoint endpoint, bytes_t buf, pkt_len_t cur_len, pkt_len_t total_len)
{
	// memset(pkt, 0, sizeof(struct fcap_pkt));

	// Check header length
	if (cur_len < HEADER_LEN)
		return FCAP_EINVAL;
	struct PacketHeader header = {};
	DecodePacketHeader(&header, buf);

	// Check version
	if (header.version != VERSION_V1)
		return FCAP_EINVAL;

	// Check body len
	pkt_len_t body_len = cur_len - HEADER_LEN;
	if (body_len != header.len)
		return FCAP_EINVAL;

	pkt->is_response = header.is_res;
	if (!pkt->is_response) {
		fcap_request_init(pkt->req, endpoint, buf, cur_len, total_len);
		struct PacketRequest pkt_req = {};
		DecodePacketRequest(&pkt_req, pkt->req->_priv.buf.buf_cur);
		fcap_pkt_buffer_free(&pkt->req->_priv.buf, HEADER_LEN);
		pkt->req->is_inbound = true;
		pkt->req->cmd = pkt_req.cmd;
		pkt->req->_priv.id = pkt_req.header.id;
		pkt->req->_priv.sent = true;
	} else {
		fcap_response_init(pkt->res, endpoint, buf, cur_len, total_len);
		struct PacketResponse pkt_res = {};
		DecodePacketResponse(&pkt_res, pkt->res->_priv.buf.buf_cur);
		pkt->res->is_inbound = true;
		pkt->res->status = pkt_res.status;
		pkt->res->_priv.id = pkt_res.header.id;
		pkt->res->_priv.sent = true;
	}
	return FCAP_OK;
}