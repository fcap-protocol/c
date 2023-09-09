#ifndef FCAP_H_
#define FCAP_H_

#include <stddef.h>
#include <stdint.h>

#define MTU 512

typedef struct fcap_packet *FPacket;

typedef enum fcap_error {
    FCAP_ENONE = 0,
    FCAP_ENOMEM,
    FCAP_EEXIST,
    FCAP_EINVAL,
    FCAP_ENOKEY,
} FERROR;

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

/* functions implemented by user */
extern FERROR fcap_send_bytes(uint8_t *bytes, size_t length);

/* Creating & Sending Packets */
FPacket fcap_init_packet();

int fcap_add_key(FPacket pkt, FKey key, FType type, uint8_t *value, size_t size);
int fcap_get_key(FPacket pkt, FKey key, uint8_t *data, size_t size);
// FERROR fcap_add_key(FPacket pkt, FKey key, FType type, void *value);

// FERROR fcap_add_key_u8(FPacket pkt, FCAP_Key key, uint8_t data);
// FERROR fcap_add_key_u16(FPacket pkt, FCAP_Key key, uint16_t data);
// FERROR fcap_add_key_i16(FPacket pkt, FCAP_Key key, int16_t data);
// FERROR fcap_add_key_u32(FPacket pkt, FCAP_Key key, uint32_t data);
// FERROR fcap_add_key_i32(FPacket pkt, FCAP_Key key, int32_t data);
// FERROR fcap_add_key_f32(FPacket pkt, FCAP_Key key, float data);
// FERROR fcap_add_key_d64(FPacket pkt, FCAP_Key key, double data);

#endif /* FCAP_H_ */
