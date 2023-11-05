#include <fcap.h>

void inline fcap_init_instance(FApp app)
{
	fcap_init_packet(&(app->out_pkt));
}

/*    Sending Functions    */

/**
 * @brief do all the middleware for this event
 * @param middleware an array of FMiddleware
 * @param num_middleware the size of the @middleware array
 * @param event the event we are processing
 * @param res a pointer to a response packet which a middleware can fill
*/
static enum handler_code fcap_do_req_middleware(const FMiddleware *middleware,
						int num_middleware,
						FEvent event,
						FPacket res)
{
	enum handler_code code = FCAP_CONTINUE;

	/* 
	 * If packet is outbound, do the middleware in order, if inbound,
	 * do them in reverse
	 */
	int i = num_middleware;
	int end = 0;
	int step = -1;

	if (event->is_outbound) {
		i = 0;
		end = num_middleware;
		step = 1;
	}

	while (i != end) {
		if (!middleware[i]->on_request)
			continue;

		code = middleware[i]->on_request(
			middleware[i]->priv, event, res);

		/* Early return if error or someone has already responded */
		if (code != FCAP_CONTINUE)
			return code;

		i += step;
	}

	return FCAP_CONTINUE;
}

static enum handler_code fcap_do_res_middleware(const FMiddleware *middleware,
						int num_middleware,
						FEvent event)
{
	enum handler_code code = FCAP_CONTINUE;

	/* 
	 * If packet is outbound, do the middleware in order, if inbound,
	 * do them in reverse
	 */
	int i = num_middleware;
	int end = 0;
	int step = -1;

	if (event->is_outbound) {
		i = 0;
		end = num_middleware;
		step = 1;
	}

	while (i != end) {
		if (!middleware[i]->on_response)
			continue;

		code = middleware[i]->on_response(middleware[i]->priv, event);

		/* Early return if error or someone has already responded */
		if (code != FCAP_CONTINUE)
			return code;

		i += step;
	}

	return FCAP_CONTINUE;
}

FError fcap_send_req(FApp app, FTransport transport)
{
	int ret;
	enum handler_code code;

	if (transport == NULL)
		return -FCAP_EINVAL;

	struct fcap_event event = {
		.is_outbound = 1,
		.pkt = &app->out_pkt,
		.transport = transport,
	};

	code = fcap_do_req_middleware(
		app->middleware, app->num_middleware, &event, &app->in_pkt);

	if (code < 0)
		return -FCAP_EINVAL;

	// TODO: handle internal loopback / short-circuiting

	ret = transport->send_bytes(transport->priv,
				    (uint8_t *)&app->out_pkt,
				    fcap_get_num_bytes(&app->out_pkt));

	/* Clean the packet after sending it */
	fcap_init_packet(&app->out_pkt);

	return ret;
}

/*    Receiving Functions    */

FError fcap_poll(FApp app)
{
	/*
	 * Note: the logic of this poll fuction is quite intricate and 
	 * the order of operations is very specific. Please take extreme
	 * care if modifying any part of this function.
	 */
	int i;
	int ret = 0;
	enum handler_code code;

	for (i = 0; i < app->num_transports; i++) {
		/* clear the in packet just incase... */
		fcap_init_packet(&app->in_pkt);

		ret = app->transports[i]->get_bytes(app->transports[i]->priv,
						    (uint8_t *)&app->in_pkt,
						    sizeof(app->in_pkt));
		if (ret < 0)
			return ret;

		/* No data :( */
		if (ret == 0)
			continue;

		struct fcap_event event = {
			.is_outbound = 0,
			.pkt = &app->in_pkt,
			.transport = app->transports[i],
		};

		/* we have a request! */
		switch (fcap_get_type(&app->in_pkt)) {
		case FCAP_REQUEST:
			/* run it through the incoming request middleware */
			code = fcap_do_req_middleware(app->middleware,
						      app->num_middleware,
						      &event,
						      &app->out_pkt);

			/* Ask the user if the want to respond */
			if (code == FCAP_CONTINUE)
				code = fcap_user_recv_req(
					app, &event, &app->out_pkt);

			/* 
			 * The req middleware or the user has handed the
			 * request, so apply the middleware and send it out
			 */
			if (code == FCAP_RESPOND) {
				/* We're about to use this, so clear it! */
				fcap_init_packet(&app->out_pkt);

				/* 
				 * Modify the event to now refer to the outgoing 
				 * response
				 */
				event.is_outbound = 1;
				event.pkt = &app->out_pkt;

				/* Copy the message ID into the response */
				app->out_pkt.header.message_id =
					app->in_pkt.header.message_id;

				/* Set the message as a response */
				fcap_set_type(&app->out_pkt, FCAP_RESPONSE);

				code = fcap_do_res_middleware(
					app->middleware,
					app->num_middleware,
					&event);

				if (code != FCAP_ABORT) {
					ret = app->transports[i]->send_bytes(
						&app->transports[i]->priv,
						(uint8_t *)&app->out_pkt,
						fcap_get_num_bytes(
							&app->out_pkt));
				}
			}

			if (code == FCAP_ABORT || ret < 0)
				return -FCAP_EINVAL;

			break;

		case FCAP_RESPONSE:
			/* We have a response to an existing request */
			code = fcap_do_res_middleware(
				app->middleware, app->num_middleware, &event);

			if (code == FCAP_CONTINUE)
				code = fcap_user_recv_res(app, &event);

			/* 
			 * If either the middleware or user aborted, then
			  * propagate this error back up 
			  */
			if (code == FCAP_ABORT)
				return -FCAP_EINVAL;

			/* 
			 * if either middleware or user returned FCAP_RESPONDED,
			 * then we don't need to do anything because it doesn't
			 * make sense to response to a response.
			 */
			break;

		default:
			return -FCAP_EINVAL;
		}
	}
	return 0;
}

inline int fcap_app_add_key_bin(FApp app, FKey key, uint8_t *data, size_t len)
{
	return fcap_add_key_bin(&app->out_pkt, key, data, len);
}

inline int fcap_app_add_key_u8(FApp app, FKey key, uint8_t value)
{
	return fcap_add_key_u8(&app->out_pkt, key, value);
}

inline int fcap_app_add_key_u16(FApp app, FKey key, uint16_t value)
{
	return fcap_add_key_u16(&app->out_pkt, key, value);
}

inline int fcap_app_add_key_i16(FApp app, FKey key, int16_t value)
{
	return fcap_add_key_i32(&app->out_pkt, key, value);
}

inline int fcap_app_add_key_i32(FApp app, FKey key, int32_t value)
{
	return fcap_add_key_i32(&app->out_pkt, key, value);
}

inline int fcap_app_add_key_i64(FApp app, FKey key, int64_t value)
{
	return fcap_add_key_i64(&app->out_pkt, key, value);
}

inline int fcap_app_add_key_f32(FApp app, FKey key, float value)
{
	return fcap_add_key_f32(&app->out_pkt, key, value);
}

inline int fcap_app_add_key_d64(FApp app, FKey key, double value)
{
	return fcap_add_key_d64(&app->out_pkt, key, value);
}

inline int fcap_app_get_key_bin(FApp app, FKey key, uint8_t *data, size_t len)
{
	return fcap_get_key_bin(&app->out_pkt, key, data, len);
}

inline int fcap_app_get_key_u8(FApp app, FKey key, uint8_t *value)
{
	return fcap_get_key_u8(&app->out_pkt, key, value);
}

inline int fcap_app_get_key_u16(FApp app, FKey key, uint16_t *value)
{
	return fcap_get_key_u16(&app->out_pkt, key, value);
}

inline int fcap_app_get_key_i16(FApp app, FKey key, int16_t *value)
{
	return fcap_get_key_i16(&app->out_pkt, key, value);
}

inline int fcap_app_get_key_i32(FApp app, FKey key, int32_t *value)
{
	return fcap_get_key_i32(&app->out_pkt, key, value);
}

inline int fcap_app_get_key_i64(FApp app, FKey key, int64_t *value)
{
	return fcap_get_key_i64(&app->out_pkt, key, value);
}

inline int fcap_app_get_key_f32(FApp app, FKey key, float *value)
{
	return fcap_get_key_f32(&app->out_pkt, key, value);
}

inline int fcap_app_get_key_d64(FApp app, FKey key, double *value)
{
	return fcap_get_key_d64(&app->out_pkt, key, value);
}
