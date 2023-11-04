#include <fcap.h>

void inline fcap_init_instance(FApp app)
{
	fcap_init_packet(&(app->pkt));
}

/*    Sending Functions    */

static int fcap_do_send_middleware(FApp app)
{
	int i;
	int ret;

	for (i = 0; i < app->num_middleware; i++) {
		if (!app->middleware[i]->on_send)
			continue;

		ret = app->middleware[i]->on_send(app->middleware[i]->priv,
						  &app->pkt);

		if (ret < 0)
			break;
	}

	return ret;
}

static int fcap_do_recv_middleware(FApp app)
{
	int i;
	int ret;

	for (i = 0; i < app->num_middleware; i++) {
		if (!app->middleware[i]->on_recv)
			continue;

		ret = app->middleware[i]->on_recv(app->middleware[i]->priv,
						  &app->pkt);

		if (ret < 0)
			break;
	}
	return 0;
}

FError fcap_send_all(FApp app)
{
	int i;
	int ret = 0;

	ret = fcap_do_send_middleware(app);
	if (ret < 0)
		return ret;

	for (i = 0; i < app->num_channels; i++) {
		/* Skip channels which can't send */
		if (!app->channels[i]->send_bytes)
			continue;

		ret = app->channels[i]->send_bytes(
			app->channels[i]->priv,
			(uint8_t *)&app->pkt,
			fcap_get_num_bytes(&app->pkt));

		if (ret < 0)
			return ret;
	}

	return ret;
}

FError fcap_send(FApp app, FChannel channel)
{
	int ret;

	if (channel == NULL)
		return -FCAP_EINVAL;

	ret = fcap_do_send_middleware(app);
	if (ret < 0)
		return ret;

	return channel->send_bytes(channel->priv,
				   (uint8_t *)&app->pkt,
				   fcap_get_num_bytes(&app->pkt));
}

/*    Receiving Functions    */

FError fcap_poll_all(FApp app)
{
	int i;
	int ret = 0;

	for (i = 0; i < app->num_channels; i++) {
		/* Ask if there is a packet avail*/
		ret = app->channels[i]->poll(app->channels[i]->priv);
		if (ret < 0)
			return ret;

		/* If there is, get it! */
		if (ret > 0) {
			ret = app->channels[i]->get_bytes(app->channels[i]->priv,
							 (uint8_t *)&app->pkt,
							 MTU);
			if (ret < 0)
				return ret;

			ret = fcap_do_recv_middleware(app);
			if (ret < 0)
				return ret;

			fcap_user_recv(app, app->channels[i]);
		}
	}

	return ret;
}

inline int fcap_app_add_key_bin(FApp app, FKey key, uint8_t *data, size_t len)
{
	return fcap_add_key(&app->pkt, key, FCAP_BINARY, data, len);
}

inline int fcap_app_add_key_u8(FApp app, FKey key, uint8_t value)
{
	return fcap_add_key(&app->pkt, key, FCAP_UINT8, &value, sizeof(value));
}

inline int fcap_app_add_key_u16(FApp app, FKey key, uint16_t value)
{
	return fcap_add_key(&app->pkt, key, FCAP_UINT16, &value, sizeof(value));
}

inline int fcap_app_add_key_i16(FApp app, FKey key, int16_t value)
{
	return fcap_add_key(&app->pkt, key, FCAP_INT16, &value, sizeof(value));
}

inline int fcap_app_add_key_i32(FApp app, FKey key, int32_t value)
{
	return fcap_add_key(&app->pkt, key, FCAP_INT32, &value, sizeof(value));
}

inline int fcap_app_add_key_i64(FApp app, FKey key, int64_t value)
{
	return fcap_add_key(&app->pkt, key, FCAP_INT64, &value, sizeof(value));
}

inline int fcap_app_add_key_f32(FApp app, FKey key, float value)
{
	return fcap_add_key(&app->pkt, key, FCAP_FLOAT, &value, sizeof(value));
}

inline int fcap_app_add_key_d64(FApp app, FKey key, double value)
{
	return fcap_add_key(&app->pkt, key, FCAP_DOUBLE, &value, sizeof(value));
}

inline int fcap_app_get_key_bin(FApp app, FKey key, uint8_t *data, size_t len)
{
	return fcap_get_key_bin(&app->pkt, key, data, len);
}

inline int fcap_app_get_key_u8(FApp app, FKey key, uint8_t *value)
{
	return fcap_get_key_u8(&app->pkt, key, value);
}

inline int fcap_app_get_key_u16(FApp app, FKey key, uint16_t *value)
{
	return fcap_get_key_u16(&app->pkt, key, value);
}

inline int fcap_app_get_key_i16(FApp app, FKey key, int16_t *value)
{
	return fcap_get_key_i16(&app->pkt, key, value);
}

inline int fcap_app_get_key_i32(FApp app, FKey key, int32_t *value)
{
	return fcap_get_key_i32(&app->pkt, key, value);
}

inline int fcap_app_get_key_i64(FApp app, FKey key, int64_t *value)
{
	return fcap_get_key_i64(&app->pkt, key, value);
}

inline int fcap_app_get_key_f32(FApp app, FKey key, float *value)
{
	return fcap_get_key_f32(&app->pkt, key, value);
}

inline int fcap_app_get_key_d64(FApp app, FKey key, double *value)
{
	return fcap_get_key_d64(&app->pkt, key, value);
}
