/**
 * @brief A function implemented by the user of this library to send bytes
 * across a transport
 * @param bytes the bytes to send
 * @param length the number of bytes to send
 * @returns number of bytes sent or any non-zero -FCAP_ERROR on failure
*/
extern int fcap_send_bytes(uint8_t *bytes, size_t length);

/**
 * @brief call this function when you receive bytes from the transport
 * @param bytes a pointer to the received bytes
 * @param length number of received bytes
 * @returns number of consumed bytes or -FCAP_ERROR on error
*/
int fcap_recv_bytes(uint8_t *bytes, size_t length);
