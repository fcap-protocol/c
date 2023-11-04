#include <fcap.h>
#include <fcap_udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define THIS_PORT 12345
#define PEER_PORT 12346
#define PEER_IP "127.0.0.1"

FCAP_CREATE_UDP_CHANNEL(my_udp);
FCAP_SET_CHANNELS(my_channels, &my_udp)

FCAP_SET_MIDDLEWARE(my_middleware)

FCAP_CREATE_APP(app, my_channels, my_middleware)

void fcap_user_recv(FApp app, FChannel channel)
{
	float val1;
	fcap_app_get_key_f32(app, KEY_A, &val1);
	printf("Server Got packet with A value: %lf\n", val1);

	int64_t val2 = 0;
	int ret = fcap_app_get_key_i64(app, KEY_AA, &val2);
	if (ret < 0) {
		printf("Key AA did not have a value :( %d\n", ret);
	} else {
		printf("Server Got packet with AA value: %ld\n", val2);
	}

	fcap_send_all(app);
}

int main()
{
	int ret;

	/* Setup a channel */
	ret = fcap_udp_setup_channel(
		&my_udp_priv, THIS_PORT, PEER_IP, PEER_PORT);
	if (ret < 0) {
		printf("Error: Failed to set up udp channel with code %d!\n",
		       ret);
		exit(1);
	}

	/* Get the first instance */
	fcap_init_instance(app);

	/* Poll everything! */
	printf("Running!\n");
	while (1) {
		ret = fcap_poll_all(app);
		if (ret < 0) {
			printf("Error: Failed to poll channels!");
			break;
		}
	}

	// fcap_udp_cleanup(&channel1);
	printf("Done\n");
}
