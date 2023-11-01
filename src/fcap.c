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
 * @brief an fcap instance
 * @param num_channels the number of setup channels
 * @param channels the array of channel fn pointers
 * @param pkt the packet buffer
*/
typedef struct fcap {
	uint8_t num_channels; /* NOTE: max 255 channels possible */
	fcap_channel_t channels[NUM_CHANNELS];
	fcap_packet_t pkt;
} fcap_app_t;

/* Statically allocated fcap instances*/
fcap_app_t fcap_apps[NUM_INSTANCES];

FApp fcap_get_instance(int id)
{
	if (id > (NUM_INSTANCES - 1)) {
		return NULL;
	}
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

/*    Sending Functions    */

FError fcap_send_all(FApp app)
{
	int i;
	int ret = 0;

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
	if (channel == NULL)
		return -FCAP_EINVAL;

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

			fcap_user_recv(app, &app->channels[i]);
		}
	}

	return ret;
}

int fcap_app_add_key_f32(FApp app, FKey key, float value)
{
	fcap_add_key_f32(&app->pkt, key, value);
}

int fcap_app_get_key_f32(FApp app, FKey key, float *value)
{
	fcap_get_key_f32(&app->pkt, key, value);
}
