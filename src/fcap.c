#include <assert.h>
#include <fcap.h>
#include <stdbool.h>
#include <string.h>

/* For debug only */
#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

#define FCAP_VERSION 0

/* Protocol defined sizes */
#define FCAP_HEADER_SIZE 2
#define FCAP_KTV_HEADER_SIZE 1
#define FCAP_KTV_BINARY_HEADER_SIZE (FCAP_KTV_HEADER_SIZE + 1)

/* Implementation defined sizes */
#define FCAP_KTVS_SIZE (MTU - FCAP_HEADER_SIZE)

static size_t fcap_type_sizes[] = {
	[FCAP_BINARY] = -1,
	[FCAP_UINT8] = sizeof(uint8_t),
	[FCAP_UINT16] = sizeof(uint16_t),
	[FCAP_INT16] = sizeof(int16_t),
	[FCAP_INT32] = sizeof(int32_t),
	[FCAP_INT64] = sizeof(int64_t),
	[FCAP_FLOAT] = sizeof(float),
	[FCAP_DOUBLE] = sizeof(double),
};

struct fcap_header {
	uint8_t num_keys : 5;
	uint8_t version : 3;
	uint8_t message_id : 7;
	uint8_t type : 1;
};

static_assert(sizeof(struct fcap_header) == FCAP_HEADER_SIZE,
	      "Header Size Mismatch!");

struct fcap_binary_value {
	uint8_t length;
	uint8_t value[0];
} __attribute__((packed));

union fcap_value {
	struct fcap_binary_value binary;
	uint8_t value[0];
} __attribute__((packed));

struct fcap_ktv {
	uint8_t key : 5;
	uint8_t type : 3;
	union fcap_value value;
} __attribute__((packed));

struct fcap_packet {
	struct fcap_header header;
	uint8_t ktv_bytes[FCAP_KTVS_SIZE];
} __attribute__((packed));

static_assert(sizeof(struct fcap_packet) == MTU,
	      "FCAP Packet too large for MTU");

static struct fcap_packet packet;

/**
 * @brief gets the size of a ktv, excluding header byte
 * @param view a pointer to the first byte of the ktv
 * @returns the size of the ktv
 * @note this function does no error checking, it assumes a valid ktv
*/
static inline size_t fcap_get_value_size(struct fcap_ktv *view)
{
	if (view->type == FCAP_BINARY)
		return view->value.binary.length;
	else
		return fcap_type_sizes[view->type];
}

/**
 * @brief gets the size of a ktv, including header byte
 * @param view a pointer to the first byte of the ktv
 * @returns the size of the ktv
 * @note this function does no error checking, it assumes a valid ktv
*/
static inline size_t fcap_get_ktv_size(struct fcap_ktv *view)
{
	if (view->type == FCAP_BINARY)
		return view->value.binary.length + FCAP_KTV_BINARY_HEADER_SIZE;
	else
		return fcap_type_sizes[view->type] + FCAP_KTV_HEADER_SIZE;
}

FPacket fcap_init_packet()
{
	FPacket pkt = &packet;

	pkt->header.message_id = 0;
	pkt->header.num_keys = 0;
	pkt->header.type = 0;
	pkt->header.version = FCAP_VERSION;

	memset(pkt->ktv_bytes, 0, FCAP_KTVS_SIZE);

	return &packet;
}

/**
 * @brief Consumes raw bytes for consumption as an FCAP packet
 * @param dest the packet to store the incoming bytes into
 * @param src the raw incoming bytes
 * @param src_len the number of bytes to consume, this cannot be greater than
 * the defined MTU
 * @returns 0 on success and all bytes were consumed or -FCAP_ERROR on failure
*/
int fcap_decode_packet(FPacket dest, uint8_t *src, int src_len)
{
	if (src_len > MTU)
		return -FCAP_EINVAL;

	/* Copy all values */
	memcpy(&dest, src, src_len);

	/* TODO: Validate packet */

	return 0;
}

/** 
 * @brief Copies the bytes from a packet into a destination buffer
 * ready to be sent across a transport
 * @param src the packet to send
 * @param dest the byte buffer to copy the packet into
 * @param dest_len the size of the @dest buffer that can be filled
 * @returns the number of bytes put into the buffer to be sent
*/
int fcap_encode_packet(FPacket src, uint8_t *dest, size_t dest_len)
{
	int key_i;
	size_t size = 0;

	for (key_i = 0; key_i < src->header.num_keys; key_i++)
		size += fcap_get_ktv_size(
			(struct fcap_ktv *)&src->ktv_bytes[size]);

	size += FCAP_HEADER_SIZE;

	if (dest_len < size)
		return -FCAP_ENOMEM;

	if (!memcpy(dest, src, size))
		return -FCAP_ENOMEM;

	return size;
}

/**
 * @brief adds a given key to a packet
 * @param pkt the packet to add the key to
 * @param key they key to the value to
 * @param type the type of the value
 * @param value a pointer to some bytes which will be copied into the packet
 * @param size the length of the value you want to copy in, this should match 
 * the protocol defined length of the @type field
 * @returns 0 on success or -FCAP_ERROR on failure
 * @note -EINVAL will be returned if adding a key that already exists
 */
int fcap_add_key(FPacket pkt, FKey key, FType type, void *value, size_t size)
{
	int key_i;
	size_t idx;
	bool found;
	size_t value_size;
	struct fcap_ktv *view;

	/* TODO: Check there is enough bytes remaining in the packet 
	to add this value safely */

	view = (struct fcap_ktv *)pkt->ktv_bytes;

	/* Find the end of the packets or if key exists */
	idx = 0;
	found = false;
	for (key_i = 0; key_i < pkt->header.num_keys; key_i++) {
		/* Check if the key already exists */
		if (view->key == key)
			return -FCAP_EINVAL;

		idx += fcap_get_ktv_size(view);
		view = (struct fcap_ktv *)&pkt->ktv_bytes[idx];
	}

	/* Copy the type */
	view->key = key;
	view->type = type;

	if (type == FCAP_BINARY) {
		value_size = size;
		view->value.binary.length = size;
		if (!memcpy(view->value.binary.value, value, value_size))
			return -FCAP_ENOMEM;

	} else {
		value_size = fcap_type_sizes[type];

		/* 
		 * Check they are passing in the correct length 
		 * for the type they asked for 
		 */
		if (size != value_size)
			return -FCAP_EINVAL;

		if (!memcpy(view->value.value, value, value_size))
			return -FCAP_ENOMEM;
	}

	pkt->header.num_keys++;

	return 0;
}

/**
 * @brief gets a specific key from the packet
 * @param pkt the packet to get the key from
 * @param key the requested key
 * @param data an output buffer for the value to be placed in
 * @param size the size of the output buffer
 * @returns the FType of the key on success or -FCAP_ERROR on failure
 * @note when the key type is binary, the first byte of the data buffer 
 * will be the length of the remaining data 
 */
int fcap_get_key(FPacket pkt, FKey key, void *data, size_t size)
{
	int key_i;
	size_t idx;
	bool found;
	size_t value_size;
	struct fcap_ktv *view;

	view = (struct fcap_ktv *)pkt->ktv_bytes;

	/* Find the end of the packets or if key exists */
	idx = 0;
	found = false;
	for (key_i = 0; key_i < pkt->header.num_keys; key_i++) {
		if (view->key == key) {
			found = true;
			break;
		}

		idx += fcap_get_ktv_size(view);
		view = (struct fcap_ktv *)&pkt->ktv_bytes[idx];
	}

	if (!found)
		return -FCAP_ENOKEY;

	value_size = fcap_get_value_size(view);
	if (view->type == FCAP_BINARY)
		value_size++;

	if (size < value_size)
		return -FCAP_ENOMEM;

	if (!memcpy(data, view->value.value, value_size))
		return -FCAP_ENOMEM;

	return view->type;
}

#ifdef DEBUG

/**
 * @brief displays info about a given ktv to stdout
 * @param bytes a pointer to the first byte of a single ktv
 * @param max_size the maximum number of bytes available to read
 */
void fcap_debug_ktv(uint8_t *bytes, size_t max_size)
{
	struct fcap_ktv *view = (struct fcap_ktv *)bytes;

	if (max_size < FCAP_KTV_HEADER_SIZE) {
		printf("Not enough bytes to read KTV Header");
		return;
	}

	printf("  Key: %d\n", view->key);
	printf("  Type: %d ", view->type);
	max_size--;

	if (max_size < fcap_type_sizes[view->type]) {
		printf("Not enough bytes, only %ld remaining.\n", max_size);
		return;
	}

	switch (view->type) {
	case FCAP_BINARY: {
		printf("(binary)\n");

		if (max_size < 1) {
			printf("Not enough bytes to read binary length!\n");
			return;
		}

		size_t len = view->value.binary.length;
		max_size--;
		printf("Length: %ld\n", len);

		if (max_size < len) {
			printf("Not enough bytes to read binary.");
			printf("Only %ld remaining bytes\n", max_size);
		}

		printf("  Binary (hex): ");
		for (int i = 0; i < len; i++) {
			printf("%02x ", view->value.binary.value[i]);
		}
		break;
	}
	case FCAP_UINT8: {
		printf("(uint8)\n");
		uint8_t *value;
		value = view->value.value;
		printf("  Value: %d\n", *value);
		break;
	}
	case FCAP_UINT16: {
		printf("(uint16)\n");
		uint16_t *value;
		value = (uint16_t *)view->value.value;
		printf("  Value: %d\n", *value);
		break;
	}
	case FCAP_INT16: {
		printf("(int16)\n");
		int16_t *value;
		value = (int16_t *)view->value.value;
		printf("  Value: %d\n", *value);
		break;
	}
	case FCAP_INT32: {
		printf("(int32)\n");
		int32_t *value;
		value = (int32_t *)view->value.value;
		printf("  Value: %d\n", *value);
		break;
	}
	case FCAP_INT64: {
		printf("(int64)\n");
		int64_t *value;
		value = (int64_t *)view->value.value;
		printf("  Value: %ld\n", *value);
		break;
	}
	case FCAP_FLOAT: {
		printf("(float)\n");
		float *value;
		value = (float *)view->value.value;
		printf("  Value: %f\n", *value);
		break;
	}
	case FCAP_DOUBLE: {
		printf("(double)\n");
		double *value;
		value = (double *)view->value.value;
		printf("  Value: %lf\n", *value);
		break;
	}
	default:
		printf("Unknown Type!\n");
		break;
	}
}

void fcap_debug_packet(FPacket pkt)
{
	int idx;
	int key_i;
	struct fcap_ktv *view;

	printf("Header:\n");
	printf("  Version: %d\n", pkt->header.version);
	printf("  Num keys: %d\n", pkt->header.num_keys);
	printf("  Message ID: %d\n", pkt->header.message_id);
	printf("  Type: %d - ", pkt->header.type);
	if (pkt->header.type == FCAP_REQUEST)
		printf("request\n");
	else
		printf("response\n");

	view = (struct fcap_ktv *)pkt->ktv_bytes;

	/* Print each KTV */
	idx = 0;
	for (key_i = 0; key_i < pkt->header.num_keys; key_i++) {
		printf("KTV [%d]\n", key_i);
		fcap_debug_ktv((uint8_t *)view, MTU - (FCAP_HEADER_SIZE + idx));

		idx += fcap_get_ktv_size(view);
		view = (struct fcap_ktv *)&pkt->ktv_bytes[idx];
	}
}

#endif /* DEBUG */
