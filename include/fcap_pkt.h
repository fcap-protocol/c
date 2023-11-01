#ifndef FCAP_PKT_H
#define FCAP_PKT_H

#include <stddef.h>
#include <stdint.h>

#define FCAP_DEBUG
#define FCAP_SAFE

#define MTU 255

typedef enum fcap_error {
	FCAP_ENONE = 0,
	FCAP_ENOMEM,
	FCAP_EEXIST,
	FCAP_EINVAL,
	FCAP_ENOKEY,
	FCAP_ETYPE,
} FError;

typedef enum fcap_type {
	FCAP_BINARY = 0,
	FCAP_UINT8,
	FCAP_UINT16,
	FCAP_INT16,
	FCAP_INT32,
	FCAP_INT64,
	FCAP_FLOAT,
	FCAP_DOUBLE,
} FType;

typedef enum fcap_pkt_type {
	FCAP_REQUEST = 0,
	FCAP_RESPONSE,
} FPktType;

typedef enum fcap_key {
	KEY_A = 0,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_AA,
	KEY_AB,
	KEY_AC,
	KEY_AD,
	KEY_AE,
	KEY_AF,
	NUM_KEYS,
} FKey;

static size_t fcap_type_sizes[] = {
	[FCAP_BINARY] = 0,
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
} __attribute__((packed));

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

typedef uint8_t ktv_bytes_t[MTU - sizeof(struct fcap_header)];

struct fcap_packet {
	struct fcap_header header;
	ktv_bytes_t ktv_bytes;
} __attribute__((packed));

typedef struct fcap_packet fcap_packet_t;
typedef struct fcap_packet *FPacket;

/* Creating & Sending Packets */
void fcap_init_packet(FPacket pkt);
int fcap_get_num_bytes(FPacket pkt);

int fcap_add_key(FPacket pkt, FKey key, FType type, void *value,
		 size_t size);
int fcap_get_key(FPacket pkt, FKey key, void *data, size_t size);

#ifdef FCAP_SAFE
int fcap_add_key_bin(FPacket pkt, FKey key, uint8_t *data, size_t len);
int fcap_add_key_u8(FPacket pkt, FKey key, uint8_t value);
int fcap_add_key_u16(FPacket pkt, FKey key, uint16_t value);
int fcap_add_key_i16(FPacket pkt, FKey key, int16_t value);
int fcap_add_key_i32(FPacket pkt, FKey key, int32_t value);
int fcap_add_key_i64(FPacket pkt, FKey key, int64_t value);
int fcap_add_key_f32(FPacket pkt, FKey key, float value);
int fcap_add_key_d64(FPacket pkt, FKey key, double value);
int fcap_get_key_bin(FPacket pkt, FKey key, uint8_t *data, size_t len);
int fcap_get_key_u8(FPacket pkt, FKey key, uint8_t *value);
int fcap_get_key_u16(FPacket pkt, FKey key, uint16_t *value);
int fcap_get_key_i16(FPacket pkt, FKey key, int16_t *value);
int fcap_get_key_i32(FPacket pkt, FKey key, int32_t *value);
int fcap_get_key_i64(FPacket pkt, FKey key, int64_t *value);
int fcap_get_key_f32(FPacket pkt, FKey key, float *value);
int fcap_get_key_d64(FPacket pkt, FKey key, double *value);
#endif /* FCAP_SAFE */

#ifdef FCAP_DEBUG
void fcap_debug_ktv(uint8_t *bytes, size_t max_size);
void fcap_debug_packet(FPacket pkt);
#endif /* FCAP_DEBUG */

#endif /* FCAP_PKT_H */
