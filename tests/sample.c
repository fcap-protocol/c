#include <fcap.h>
#include <stdint.h>
#include <stdio.h>

void print_bytes(void *bytes, size_t len)
{
	uint8_t *bytes_arr = bytes;

	printf("Bytes:\n");
	for (int i = 0; i < len; i++) {
		printf("[%d]: 0x%02x\n", i, bytes_arr[i]);
	}
}

int main()
{
	int ret;
	FPacket pkt = fcap_init_packet();

	uint8_t value1 = 13;
	ret = fcap_add_key(pkt, KEY_C, FCAP_UINT8, &value1, sizeof(value1));
	if (ret < 0) {
		printf("Add key 1 returned %d\n", ret);
		return -1;
	}

	uint8_t value2 = 42;
	ret = fcap_add_key(pkt, KEY_B, FCAP_UINT8, &value2, sizeof(value2));
	if (ret < 0) {
		printf("Add key 2 returned %d\n", ret);
		return -1;
	}

	print_bytes(pkt, 10);
	fcap_debug_packet(pkt);

	uint8_t recv1;
	ret = fcap_get_key(pkt, KEY_C, &recv1, sizeof(recv1));
	if (ret < 0) {
		printf("Get key 1 returned %d\n", ret);
		return -1;
	}

	printf("Sent1: %d\n", value1);
	printf("Recv1: %d\n", recv1);

	uint8_t recv2;
	ret = fcap_get_key(pkt, KEY_B, &recv2, sizeof(recv2));
	if (ret < 0) {
		printf("Get key 2 returned %d\n", ret);
		return -1;
	}

	printf("Sent2: %d\n", value2);
	printf("Recv2: %d\n", recv2);
}
