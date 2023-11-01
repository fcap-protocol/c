#ifndef FCAP_H
#define FCAP_H

#include <fcap_pkt.h>

#define NUM_INSTANCES 1
#define NUM_CHANNELS 5

typedef struct fcap *FApp;
typedef struct fcap_channel *FChannel;

/**
 * @brief get a reference to a statically created fcap instances
 * @param id the id of statically created fcap instance. This has a 
 * range of [0, NUM_INSTANCES)
 * @returns a reference to the instance or NULL if the instance doesn't exist
 * @note Users can adjust the number of instances which are statically created by 
 * changing the value of NUM_INSTANCES, only 1 is created by default.
*/
FApp fcap_get_instance(int id);

/**
 * @brief setup a channel to send packets on
 * @param app the fcap application instance to add the channel to
 * @param priv any private context data the channel needs to maintain
 * @param send_bytes a function to call to send bytes out on this channel, 
 * returns 0 on success or -errno on failure
 * @param poll a function which returns non-zero if there is a packet to read,
 * zero if no packet or -errno if error checking. This function should NEVER block
 * @param get_bytes a function which will load the bytes array with new bytes from
 * the channel. Returns the number of bytes received or -errno on failure. 
 * This fuction should NEVER block
 * @returns a reference to the fcap channel or NULL on failure.
*/
FChannel
fcap_add_channel(FApp app,
		 void *priv,
		 int (*send_bytes)(void *priv, uint8_t *bytes, size_t length),
		 int (*poll)(void *priv),
		 int (*get_bytes)(void *priv, uint8_t *bytes, size_t length));

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

int fcap_app_add_key_f32(FApp app, FKey key, float value);
int fcap_app_get_key_f32(FApp app, FKey key, float *value);

#endif /* FCAP_H */
