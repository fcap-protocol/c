#include <fcap.h>
#include <fcap_udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// void print_bytes(void *bytes, size_t len)
// {
// 	uint8_t *bytes_arr = bytes;

// 	printf("Bytes:\n");
// 	for (int i = 0; i < len; i++) {
// 		printf("[%d]: 0x%02x '%c'\n", i, bytes_arr[i], bytes_arr[i]);
// 	}
// }

// #define SIZE 64

// fcap_udp_t transport1;

int main()
{
	// int ret;
	// ret = fcap_udp_setup_transport(&transport1, 12345, "127.0.0.1", 8080);
	// if (ret < 0) {
	// 	printf("Error setting up udp transport: %d\n", ret);
	// 	exit(1);
	// }

	// uint8_t bytes[SIZE];


	// printf("Running!\n");
	// while (1) {
	// 	if (fcap_udp_poll(&transport1) > 0) {
			
	// 		printf("Got bytes!\n");

	// 		ret = fcap_udp_get_bytes(&transport1, bytes, SIZE);

	// 		if (ret > 0) {
	// 			print_bytes(bytes, SIZE);
	// 		} 

	// 		ret = fcap_udp_send_bytes(&transport1, bytes, SIZE);
	// 		if (ret < 0) {
	// 			printf("error sending!\n");
	// 		} else {
	// 			printf("Sent %d bytes!\n", ret);
	// 		}
	// 		break;
	// 	}
	// }
	// fcap_udp_cleanup(&transport1);
	// printf("Done\n");
}
