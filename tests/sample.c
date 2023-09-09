#include <fcap.h>
#include <stdint.h>
#include <stdio.h>

void print_bytes(uint8_t *bytes, size_t len)
{
    printf("Bytes:\n");
    for (int i = 0; i < len; i++) {
        printf("[%d]: 0x%02x\n", i, bytes[i]);
    }
    printf("\n");
}

int main() {

    int ret;
    FPacket pkt = fcap_init_packet();
    uint8_t *bytes = (uint8_t *)pkt;

    uint8_t value = 13;
    ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &value, sizeof(value));
    if (ret < 0) {
        printf("Add key returned %d\n", ret);
        return -1;
    }

    print_bytes(bytes, 32);

    uint8_t recv;
    ret = fcap_get_key(pkt, KEY_A, &recv, sizeof(recv));
    if (ret < 0) {
        printf("Get key returned %d\n", ret);
        return -1;
    }

    printf("Sent: %d\n", value);
    printf("Recv: %d\n", recv);

}
