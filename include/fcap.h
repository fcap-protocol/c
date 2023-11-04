#ifndef FCAP_H
#define FCAP_H

#include <fcap_pkt.h>

/**
 * @brief all info needed to manage and use a transport channel
 * @param priv any private context data the channel needs to maintain
 * @param send_bytes a function to call to send bytes out on this channel, 
 * returns 0 on success or -errno on failure
 * @param poll a function which returns non-zero if there is a packet to read,
 * zero if no packet or -errno if error checking. This function should NEVER block
 * @param get_bytes a function which will load the bytes array with new bytes from
 * the channel. Returns the number of bytes received or -errno on failure. 
 * This fuction should NEVER block
*/
struct fcap_channel {
	void *priv;
	int (*send_bytes)(void *priv, uint8_t *bytes, size_t length);
	int (*poll)(void *priv);
	int (*get_bytes)(void *priv, uint8_t *bytes, size_t length);
};
typedef struct fcap_channel *FChannel;

/**
 * @brief all info needed to manage and use a middleware layer
 * @param priv any private context data the channel needs to maintain
 * @param send_bytes a function to call to send bytes out on this channel, 
 * returns 0 on success or -errno on failure
 * @param poll a function which returns non-zero if there is a packet to read,
 * zero if no packet or -errno if error checking. This function should NEVER block
 * @param get_bytes a function which will load the bytes array with new bytes from
 * the channel. Returns the number of bytes received or -errno on failure. 
 * This fuction should NEVER block
 * @note these functions will modify the packets in place
*/
struct fcap_middleware {
	void *priv;
	int (*on_send)(void *priv, FPacket pkt);
	int (*on_recv)(void *priv, FPacket pkt);
};
typedef struct fcap_middleware *FMiddleware;

/**
 * @brief an fcap instance
 * @param num_channels the number of setup channels
 * @param num_middleware number of middleware patterns
 * @param channels an array of channel pointers
 * @param middlewares an array of middleware pointers
 * @param pkt the packet buffer
*/
struct fcap {
	const uint8_t num_channels;
	const uint8_t num_middleware;
	const FChannel *channels;
	const FMiddleware *middleware;
	fcap_packet_t pkt;
};
typedef struct fcap *FApp;

#define FCAP_CREATE_APP(name, channels_in, middleware_in)                      \
	struct fcap name##_internal = { .num_channels = channels_in##_size,    \
					.num_middleware =                      \
						middleware_in##_size,          \
					.channels = channels_in,               \
					.middleware = middleware_in,           \
					.pkt = {} };                           \
	const FApp name = &name##_internal;

#define FCAP_SET_CHANNELS(name, ...)                                           \
	const FChannel name[] = { __VA_ARGS__ };                               \
	const int name##_size = sizeof(name) / sizeof(FChannel);

#define FCAP_SET_MIDDLEWARE(name, ...)                                         \
	const FMiddleware name[] = { __VA_ARGS__ };                            \
	const int name##_size = sizeof(name) / sizeof(FMiddleware);

/**
 * @brief initialised a statically created fcap instance
 * This will reset the instance if called more than once
 * @param app the application instance to initialise
*/
void fcap_init_instance(FApp app);

/**
 * @brief sends the packet out on all channels
*/
FError fcap_send_all(FApp app);

/**
 * @brief sends the packet out on specific channel
*/
FError fcap_send(FApp app, FChannel channel);

/**
 * @brief loop which asks each channel if there is any data available to read
 * @param app the fcap app to check for data
 * @return 0 on success or -errno on failure
*/
FError fcap_poll_all(FApp app);

/**
 * @brief a function for the user to implement.
 * This function will be called when any channel receives a packet.
 * The user should then use the get and set methods on the underlying packet
 * @param app the application the packet came from
 * @param channel the channel the packet came from
 * 
*/
extern void fcap_user_recv(FApp app, FChannel channel);

int fcap_app_add_key_bin(FApp app, FKey key, uint8_t *data, size_t len);
int fcap_app_add_key_u8(FApp app, FKey key, uint8_t value);
int fcap_app_add_key_u16(FApp app, FKey key, uint16_t value);
int fcap_app_add_key_i16(FApp app, FKey key, int16_t value);
int fcap_app_add_key_i32(FApp app, FKey key, int32_t value);
int fcap_app_add_key_i64(FApp app, FKey key, int64_t value);
int fcap_app_add_key_f32(FApp app, FKey key, float value);
int fcap_app_add_key_d64(FApp app, FKey key, double value);
int fcap_app_get_key_bin(FApp app, FKey key, uint8_t *data, size_t len);
int fcap_app_get_key_u8(FApp app, FKey key, uint8_t *value);
int fcap_app_get_key_u16(FApp app, FKey key, uint16_t *value);
int fcap_app_get_key_i16(FApp app, FKey key, int16_t *value);
int fcap_app_get_key_i32(FApp app, FKey key, int32_t *value);
int fcap_app_get_key_i64(FApp app, FKey key, int64_t *value);
int fcap_app_get_key_f32(FApp app, FKey key, float *value);
int fcap_app_get_key_d64(FApp app, FKey key, double *value);

#endif /* FCAP_H */
