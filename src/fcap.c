#include <assert.h>
#include <fcap.h>
#include <string.h>

#define FCAP_VERSION 0

#define FCAP_HEADER_SIZE 2
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

static_assert(sizeof(struct fcap_header) == FCAP_HEADER_SIZE, "Header Size Mismatch!");

struct fcap_binary_value {
    uint8_t length;
    uint8_t value[];
};

union fcap_value {
    struct fcap_binary_value bin;
    uint8_t value[];
};

struct fcap_ktv {
    uint8_t key : 5;
    uint8_t type : 3;
    union fcap_value;
};

union fcap_ktvs {
    uint8_t bytes[FCAP_KTVS_SIZE];
    struct fcap_ktv ktv[];
};
static_assert(sizeof(union fcap_ktvs) == FCAP_KTVS_SIZE, "FCAP KTVS wrong size");

struct fcap_packet {
    struct fcap_header header;
    union fcap_ktvs ktvs;
} __attribute__((packed));
static_assert(sizeof(struct fcap_packet) == MTU, "FCAP Packet too large for MTU");

static struct fcap_packet packet;

FPacket fcap_init_packet() {

    FPacket pkt = &packet;

    pkt->header.message_id = 0;
    pkt->header.num_keys = 0;
    pkt->header.type = 0;
    pkt->header.version = FCAP_VERSION;

    memset(pkt->ktvs.bytes, 0, sizeof(union fcap_ktvs));

    return &packet;
}

void fcap_packet_view_init(struct fcap_packet_view *view, uint8_t *src) {
    view->header = (struct fcap_header *)src;
    view->kt_headers = (struct fcap_kt_header *)&src[FCAP_HEADER_SIZE];
    view->values = &src[FCAP_HEADER_SIZE + view->header->num_keys];
}

// TODO: copy pattern from encode
FERROR fcap_decode_packet(FPacket dest, uint8_t *src, int src_len) {
    int i;
    size_t values_size;
    struct fcap_packet_view view;

    /* Check the packet is the smallest possible packet */
    if (src_len < FCAP_SMALLEST_PKT_SIZE)  // FIXME: this is not true (can send 0 ktv's), i know, just not starting there :p
        return -FCAP_EINVAL;

    /* Initialise the first KTV view*/
    fcap_packet_view_init(&view, src);

    /* Copy the header in */
    dest->header = *view.header;

    /* Copy all KTs */
    memcpy(dest->kt_headers, view.kt_headers, view.header->num_keys * FCAP_KT_SIZE);

    /* Calculate the size of all values */
    values_size = 0;
    for (i = 0; i < view.header->num_keys; i++) {
        values_size += fcap_type_sizes[view.kt_headers[i].type];
    }

    /* Copy all values */
    memcpy(dest->values, view.values, values_size);

    return -FCAP_ENONE;
}

// ty brb copy
// 3. check dest_len > len of values then push those
FERROR fcap_encode_packet(FPacket src, uint8_t *dest, size_t dest_len) {
    int i;
    size_t size;

    // 1. check dest_len > 2 to push the header
    size = FCAP_HEADER_SIZE;
    if (dest_len < size)
        return -FCAP_ENOMEM;
    memcpy(dest, &src->header, FCAP_HEADER_SIZE);
    dest += size;
    dest_len -= size;

    // 2. check dest_len > number of kt headers and push them and count the size of values
    size = src->header.num_keys * FCAP_KT_SIZE;
    if (dest_len < size)
        return -FCAP_ENOMEM;
    memcpy(dest, &src->kt_headers, size);
    dest += size;
    dest_len -= size;

    // 3a. count size of all values
    size = 0;
    for (i = 0; i < src->header.num_keys; i++) {
        size += fcap_type_sizes[src->kt_headers[i].type];
    }

    if (dest_len < size)
        return -FCAP_ENOMEM;
    memcpy(dest, src->values, size);

    return -FCAP_ENONE;
}

// FERROR fcap_show_packet(FPacket pkt) {
//     int i;
//     struct fcap_packet_view view;

//     /* Initialise the first KTV view*/
//     view.header = pkt->view.kt_headers = (struct fcap_kt_header *)&data[FCAP_HEADER_SIZE];
//     view.value = &data[FCAP_HEADER_SIZE + view.header->num_keys];

//     return FCAP_ENONE;
// }

FERROR fcap_add_key(FPacket pkt, FKey key, uint8_t *data, size_t length);
{

}
