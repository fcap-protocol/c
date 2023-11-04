#include <fcap.h>
#include <fcap_udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define THIS_PORT 12346
#define PEER_PORT 12345
#define PEER_IP "127.0.0.1"

FCAP_CREATE_UDP_CHANNEL(my_udp);
FCAP_SET_CHANNELS(my_channels, &my_udp)

FCAP_SET_MIDDLEWARE(my_middleware)

FCAP_CREATE_APP(app, my_channels, my_middleware)

int my_on_send(void *priv, FPacket pkt)
{
	int ret;
	if (!fcap_has_key(pkt, KEY_AA)) {
		ret = fcap_add_key_i64(pkt, KEY_AA, 58);
		if (ret < 0) {
			printf("Error: could not add i64 %d\n", ret);
		}
	}
	return 0;
}

void fcap_user_recv(FApp app, FChannel channel)
{
	float val;
	fcap_app_get_key_f32(app, KEY_A, &val);

	exit(1);
}

int main()
{
	int ret;

	/* Setup a channel */
	ret = fcap_udp_setup_channel(&my_udp_priv, THIS_PORT, PEER_IP, PEER_PORT);
	if (ret < 0) {
		printf("Error: Failed to set up udp channel with code %d!\n", ret);
		exit(1);
	}

	/* Get the first instance */
	fcap_init_instance(app);

	// c1
	fcap_app_add_key_f32(app, KEY_A, 12.34);
	fcap_send_all(app);

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
