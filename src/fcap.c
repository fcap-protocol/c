#include <fcap.h>

/**
 * @brief all info needed to manage and use a transport channel
 * @param priv private data to the channel, typically used for context
 * @param send_bytes a function to call to send bytes out on this channel, 
 * returns 0 on success or -errno on failure
 * @param poll a function which returns non-zero if there is a packet to read,
 * zero if no packet or -errno if error checking
 * @param get_bytes a function which will load the bytes array with new bytes from
 * the channel. Returns 0 on success or -errno on error
*/
typedef struct fcap_channel {
	void *priv;
	int (*send_bytes)(void *priv, uint8_t *bytes, size_t length);
	int (*poll)(void *priv);
	int (*get_bytes)(void *priv, uint8_t *bytes, size_t length);

} fcap_channel_t;

/**
 * @brief all info needed to manage and use a middleware layer
 * @param priv private data to the middleware
 * @param on_send the function which is called with the packet before 
 * transmitting on a channel. returns 0 on success or -errno on failure
 * @param on_recv the function which is called after bytes have been received
 * but before the user is alerted to it
 * @note these functions will modify the packets in place
*/
typedef struct fcap_middleware {
	void *priv;
	int (*on_send)(void *priv, FPacket pkt);
	int (*on_recv)(void *priv, FPacket pkt);
} fcap_middleware_t;

/**
 * @brief an fcap instance
 * @param num_channels the number of setup channels
 * @param channels the array of channel fn pointers
 * @param pkt the packet buffer
*/
typedef struct fcap {
	uint8_t num_channels; /* NOTE: max 255 channels possible */
	uint8_t num_middleware; /* NOTE: max 255 middleware possible */
	fcap_channel_t channels[NUM_CHANNELS];
	fcap_middleware_t middlewares[NUM_MIDDLEWARE];
	fcap_packet_t pkt;
} fcap_app_t;

/* Statically allocated fcap instances*/
fcap_app_t fcap_apps[NUM_INSTANCES];

FApp fcap_init_instance(int id)
{
	if (id > (NUM_INSTANCES - 1)) {
		return NULL;
	}

	fcap_apps[id].num_channels = 0;
	fcap_apps[id].num_middleware = 0;

	fcap_init_packet(&fcap_apps[id].pkt);

	return &fcap_apps[id];
}

FChannel
fcap_add_channel(FApp app,
		 void *priv,
		 int (*send_bytes)(void *priv, uint8_t *bytes, size_t length),
		 int (*poll)(void *priv),
		 int (*get_bytes)(void *priv, uint8_t *bytes, size_t length))
{
	if (app->num_channels == NUM_CHANNELS)
		return NULL;

	/* We use the number of channels as the index into the array */
	app->channels[app->num_channels].priv = priv;
	app->channels[app->num_channels].send_bytes = send_bytes;
	app->channels[app->num_channels].poll = poll;
	app->channels[app->num_channels].get_bytes = get_bytes;

	app->num_channels++;

	return &app->channels[app->num_channels - 1];
}

int fcap_add_middleware(FApp app,
			void *priv,
			int (*on_send)(void *priv, FPacket pkt),
			int (*on_recv)(void *priv, FPacket pkt))
{
	if (app->num_middleware == NUM_MIDDLEWARE)
		return -FCAP_ENOMEM;

	app->middlewares[app->num_middleware].priv = priv;
	app->middlewares[app->num_middleware].on_send = on_send;
	app->middlewares[app->num_middleware].on_recv = on_recv;

	app->num_middleware++;

	return 0;
}

/*    Sending Functions    */

static int fcap_do_send_middleware(FApp app)
{
	int i;
	int ret;

	for (i = 0; i < app->num_middleware; i++) {
		if (!app->middlewares[i].on_send)
			continue;

		ret = app->middlewares[i].on_send(app->middlewares[i].priv,
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
		if (!app->middlewares[i].on_recv)
			continue;

		ret = app->middlewares[i].on_recv(app->middlewares[i].priv,
						  &app->pkt);

		if (ret < 0)
			break;
	}
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
		if (app->channels[i].send_bytes == NULL)
			continue;

		ret = app->channels[i].send_bytes(
			app->channels[i].priv,
			(uint8_t *)&app->pkt,
			fcap_get_num_bytes(&app->pkt));

		if (ret < 0)
			return ret;
	}

	return ret;
}

FError fcap_send(FApp app, FChannel channel)
{
	int i;
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
		ret = app->channels[i].poll(app->channels[i].priv);
		if (ret < 0)
			return ret;

		/* If there is, get it! */
		if (ret > 0) {
			ret = app->channels[i].get_bytes(app->channels[i].priv,
							 (uint8_t *)&app->pkt,
							 MTU);
			if (ret < 0)
				return ret;

			ret = fcap_do_recv_middleware(app);
			if (ret < 0)
				return ret;

			fcap_user_recv(app, &app->channels[i]);
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
