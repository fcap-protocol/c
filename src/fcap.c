#include <assert.h>
#include <fcap.h>
#include <stdbool.h>
#include <string.h>

/* For debug only */
#define DEBUG

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

static_assert(sizeof(struct fcap_header) == FCAP_HEADER_SIZE, "Header Size Mismatch!");

struct fcap_binary_value {
    uint8_t length;
    uint8_t value[0];
} __attribute__((packed));

union fcap_value {
    struct fcap_binary_value binary;
    uint8_t value[0];
} __attribute__((packed));

struct fcap_ktv_view {
    uint8_t key : 5;
    uint8_t type : 3;
    union fcap_value value;
} __attribute__((packed));

struct fcap_packet {
    struct fcap_header header;
    uint8_t ktv_bytes[FCAP_KTVS_SIZE];
} __attribute__((packed));

static_assert(sizeof(struct fcap_packet) == MTU, "FCAP Packet too large for MTU");

static struct fcap_packet packet;

FPacket fcap_init_packet() {
    FPacket pkt = &packet;

    pkt->header.message_id = 0;
    pkt->header.num_keys = 0;
    pkt->header.type = 0;
    pkt->header.version = FCAP_VERSION;

    memset(pkt->ktv_bytes, 0, FCAP_KTVS_SIZE);

    return &packet;
}

#ifdef DEBUG
/**
 * @brief displays info about a given ktv
 * @returns 0 on success or -FCAP_ERROR on failure
 */
void fcap_debug_ktv(uint8_t *bytes, size_t max_size) {
    struct fcap_ktv_view *view = (struct fcap_ktv_view *)bytes;

    if (max_size < 1) {
        printf("Not enough bytes to read KTV Header");
        return;
    }
    printf("Key: %d\n", view->key);
    printf("Type: %d ", view->type);
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

            printf("Binary (hex): ");
            for (int i = 0; i < len; i++) {
                printf("%02x ", view->value.binary.value[i]);
            }
            break;
        }
        case FCAP_UINT8: {
            printf("(uint8)\n");
            uint8_t *value;
            value = view->value.value;
            printf("Value: %d\n", *value);
            break;
        }
        case FCAP_UINT16: {
            printf("(uint16)\n");
            uint16_t *value;
            value = (uint16_t *)view->value.value;
            printf("Value: %d\n", *value);
            break;
        }
        case FCAP_INT16: {
            printf("(int16)\n");
            int16_t *value;
            value = (int16_t *)view->value.value;
            printf("Value: %d\n", *value);
            break;
        }
        case FCAP_INT32: {
            printf("(int32)\n");
            int32_t *value;
            value = (int32_t *)view->value.value;
            printf("Value: %d\n", *value);
            break;
        }
        case FCAP_INT64: {
            printf("(int64)\n");
            int64_t *value;
            value = (int64_t *)view->value.value;
            printf("Value: %ld\n", *value);
            break;
        }
        case FCAP_FLOAT: {
            printf("(float)\n");
            float *value;
            value = (float *)view->value.value;
            printf("Value: %f\n", *value);
            break;
        }
        case FCAP_DOUBLE: {
            printf("(double)\n");
            double *value;
            value = (double *)view->value.value;
            printf("Value: %lf\n", *value);
            break;
        }
        default:
            printf("Unknown Type!\n");
            break;
    }
}
#endif /* DEBUG */


// FERROR fcap_decode_packet(FPacket dest, uint8_t *src, int src_len) {
//     int i;
//     size_t values_size;
//     struct fcap_packet_view view;

//     /* Initialise the first KTV view*/
//     fcap_packet_view_init(&view, src);

//     /* Copy the header in */
//     dest->header = *view.header;

//     /* Copy all KTs */
//     memcpy(dest->kt_headers, view.kt_headers, view.header->num_keys * FCAP_KT_SIZE);

//     /* Calculate the size of all values */
//     values_size = 0;
//     for (i = 0; i < view.header->num_keys; i++) {
//         values_size += fcap_type_sizes[view.kt_headers[i].type];
//     }

//     /* Copy all values */
//     memcpy(dest->values, view.values, values_size);

//     return -FCAP_ENONE;
// }

// // ty brb copy
// // 3. check dest_len > len of values then push those
// FERROR fcap_encode_packet(FPacket src, uint8_t *dest, size_t dest_len) {
//     int i;
//     size_t size;

//     // 1. check dest_len > 2 to push the header
//     size = FCAP_HEADER_SIZE;
//     if (dest_len < size)
//         return -FCAP_ENOMEM;
//     memcpy(dest, &src->header, FCAP_HEADER_SIZE);
//     dest += size;
//     dest_len -= size;

//     // 2. check dest_len > number of kt headers and push them and count the size of values
//     size = src->header.num_keys * FCAP_KT_SIZE;
//     if (dest_len < size)
//         return -FCAP_ENOMEM;
//     memcpy(dest, &src->kt_headers, size);
//     dest += size;
//     dest_len -= size;

//     // 3a. count size of all values
//     size = 0;
//     for (i = 0; i < src->header.num_keys; i++) {
//         size += fcap_type_sizes[src->kt_headers[i].type];
//     }

//     if (dest_len < size)
//         return -FCAP_ENOMEM;
//     memcpy(dest, src->values, size);

//     return -FCAP_ENONE;
// }

// FERROR fcap_show_packet(FPacket pkt) {
//     int i;
//     struct fcap_packet_view view;

//     /* Initialise the first KTV view*/
//     view.header = pkt->view.kt_headers = (struct fcap_kt_header *)&data[FCAP_HEADER_SIZE];
//     view.value = &data[FCAP_HEADER_SIZE + view.header->num_keys];

//     return FCAP_ENONE;
// }

/**
 * @todo: What happens if we add a key that already exists?
 * This impil will exit out for for, but later it should change it
 * @brief adds a given key to a packet
 * @param pkt the packet to add the key to
 * @param key they key to the value to
 * @param type the type of the value
 * @param value a pointer to some bytes which will be copied into the packet
 * @param size the length of the value you want to copy in, this should match the
 * protocol defined length of the @type field
 * @returns 0 on success or -FCAP_ERROR on failure
 */
int fcap_add_key(FPacket pkt, FKey key, FType type, uint8_t *value, size_t size) {
    int count;
    struct fcap_ktv_view *view;
    size_t idx = 0;
    bool found = false;

    view = (struct fcap_ktv_view *)&pkt->ktv_bytes[0];

    /* Find the end of the packets or if key exists */
    for (count = 0; count < pkt->header.num_keys; count++) {
        /* Check if the key already exists */
        if (view->key == key) {
            return -FCAP_EINVAL;
        }

        if (view->type == FCAP_BINARY)
            idx += view->value.binary.length + FCAP_KTV_BINARY_HEADER_SIZE;
        else
            idx += fcap_type_sizes[view->type] + FCAP_KTV_HEADER_SIZE;

        view = (struct fcap_ktv_view *)&pkt->ktv_bytes[idx];
    }

    /* Copy the type */
    view->key = key;
    view->type = type;

    /* Check they are passing in the correct length for the type they asked for */
    if (size != fcap_type_sizes[type])
        return -FCAP_EINVAL;

    if (!memcpy(view->value.value, value, fcap_type_sizes[view->type]))
        return -FCAP_ENOMEM;

    pkt->header.num_keys++;

    return FCAP_ENONE;
}

/**
 * @brief gets a specific key from the packet
 * @param pkt the packet to get the key from
 * @param key the requested key
 * @param data an output buffer for the value to be placed in
 * @param size the size of the output buffer
 * @returns the FType of the key on success or -FCAP_ERROR on failure
 */
int fcap_get_key(FPacket pkt, FKey key, uint8_t *data, size_t size) {
    int count;
    struct fcap_ktv_view *view;
    size_t idx = 0;
    bool found = false;

    view = (struct fcap_ktv_view *)&pkt->ktv_bytes[idx];

    /* Find the end of the packets or if key exists */
    for (count = 0; count < pkt->header.num_keys; count++) {
        if (view->key == key) {
            found = true;
            break;
        }

        if (view->type == FCAP_BINARY)
            idx += view->value.binary.length + FCAP_KTV_BINARY_HEADER_SIZE;
        else
            idx += fcap_type_sizes[view->type] + FCAP_KTV_HEADER_SIZE;

        view = (struct fcap_ktv_view *)&pkt->ktv_bytes[idx];
    }

    if (!found)
        return -FCAP_ENOKEY;

    // FIXME: make this nicer
    if (view->type == FCAP_BINARY) {
        if (size < view->value.binary.length) {
            return -FCAP_EINVAL;
        }
    } else {
        if (size < fcap_type_sizes[view->type])
            return -FCAP_ENOMEM;
    }

    if (!memcpy(data, view->value.value, fcap_type_sizes[view->type]))
        return -FCAP_ENOMEM;

    return view->type;
}
