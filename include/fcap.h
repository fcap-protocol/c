#ifndef FCAP_H
#define FCAP_H

#include <fcap_pkt.h>

/* Standard FCAP port == 1434 */
#define FCAP_PORT (1024 + 'F' + 'C' + 'A' + 'P')

/**
 * @brief all info needed to manage and use a transport transport
 * @param priv any private context data the transport needs to maintain
 * @param get_bytes a function which will load the bytes array with new bytes 
 * from the transport. Returns the number of bytes received (0 if no data ready)
 * or -errno on failure. This fuction should never block
 * @param send_bytes a function to call to send bytes out on this transport, 
 * returns number of bytes sent or -errno on failure. 
*/
struct fcap_transport {
	void *priv;
	int (*get_bytes)(void *priv, uint8_t *bytes, size_t length);
	int (*send_bytes)(void *priv, uint8_t *bytes, size_t length);
};
typedef struct fcap_transport *FTransport;

/**
 * @brief context object to pass around with a packet
 * @param transport the transport the message came from
 * @param pkt a pointer to the packet buffer
 * @param is_outbound is this packet being sent out of the device (true) or 
 * is from an external peer and inbound (false)
*/
struct fcap_event {
	FTransport transport;
	FPacket pkt;
	uint8_t is_outbound;
};
typedef struct fcap_event *FEvent;

/**
 * @brief codes for packet handlers
*/
enum handler_code {
	FCAP_ABORT = -1,
	FCAP_CONTINUE = 0,
	FCAP_RESPOND = 1,
};

/**
 * @brief all info needed to manage and use a middleware layer
 * @param priv any private context data the transport needs to maintain
 * @param on_request handles request events both inbound and outbound from the 
 * transports perspectives. Request handers can 'short circuit' in both 
 * direction and automatically response to the request. Use the following return
 * codes to indicate how the fcap-stack should progress after this middleware
 * FCAP_ABORT = There is an issues with this packet, drop is here 
 * FCAP_CONTINUE = The middleware made changes as needed to the packet and 
 * should continue as normal with other middleware or returning to the user
 * FCAP_RESPOND = the middleware has filled out a response packet and should
 * be sent straight back to the requesters - do not pass go, do not collect $200
 * @param on_response handles response events both inbound and outbound, from 
 * the transports perspective. returns an enum handler_code. 
 * //TODO: fill the return codes...
 * @note these functions can modify the packets in place or automatically 
 * handles a request.
*/
struct fcap_middleware {
	void *priv;
	enum handler_code (*on_request)(void *priv, FEvent event, FPacket res);
	enum handler_code (*on_response)(void *priv, FEvent event);
};
typedef struct fcap_middleware *FMiddleware;

/**
 * @brief an fcap instance
 * @param num_transports the number of setup transports
 * @param num_middleware number of middleware patterns
 * @param transports an array of transport pointers
 * @param middlewares an array of middleware pointers
 * @param out_pkt the tx packet buffer
 * @param in_pkt the rx_packet buffer
*/
struct fcap {
	const uint8_t num_transports;
	const uint8_t num_middleware;
	const FTransport *transports;
	const FMiddleware *middleware;
	struct fcap_packet out_pkt;
	struct fcap_packet in_pkt;
};
typedef struct fcap *FApp;

#define FCAP_CREATE_APP(name, transports_in, middleware_in)                    \
	struct fcap name##_internal = {                                        \
		.num_transports = transports_in##_size,                        \
		.num_middleware = middleware_in##_size,                        \
		.transports = transports_in,                                   \
		.middleware = middleware_in,                                   \
		.out_pkt = {},                                                 \
		.in_pkt = {},                                                  \
	};                                                                     \
	const FApp name = &name##_internal;

#define FCAP_SET_TRANSPORTS(name, ...)                                         \
	const FTransport name[] = { __VA_ARGS__ };                             \
	const int name##_size = sizeof(name) / sizeof(FTransport);

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
 * @brief sends the packet out on specific transport
*/
FError fcap_send_req(FApp app, FTransport transport);

/**
 * @brief loop which asks each transport if there is any data available to read
 * @param app the fcap app to check for data
 * @returns 0 on success or -errno on failure
 * @note Ownership of the packet buffer is lost when yielding to this function
 * call. I.e. any built packets will be reset after calling this fn.
 * Use it or lose it baby!
*/
FError fcap_poll(FApp app);

/**
 * @brief a the default callback when a request is received
 * This function will be called when any transport receives a request packet
 * and no middleware has handed it.
 * @param app the application the packet came from
 * @param event the event info, including the request packet
 * @param res the response packet to fill if the user wants to respond to this
 * request 
 * @returns a handler code indicating to the core if the response should be sent
 * return FCAP_RESPOND to send the response, FCAP_CONTINUE to acknowledge but
 * do nothing or FCAP_ABORT if there is a critical issue.
*/
extern enum handler_code
fcap_user_recv_req(FApp app, FEvent event, FPacket res);

/**
 * @brief the default callback when a response is received and no other
 * middleware has handled it.
 * @param app the application which the response came from
 * @param event the event data including the response packet
 * @returns a handler code indicating how the response was delt with.
 * FCAP_ABORT indicates an error and will bubble this back up to other parts of
 * of the stack. FCAP_CONTINUE means you got the response and handled it.
 * FCAP_RESPONDED has the same effect as FCAP_CONTINUE.
*/
extern enum handler_code fcap_user_recv_res(FApp app, FEvent event);

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
