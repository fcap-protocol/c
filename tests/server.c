#include <fcap.h>
#include <fcap_udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define THIS_PORT 12345
#define PEER_PORT 12346
#define PEER_IP "127.0.0.1"

fcap_udp_t udp;

void fcap_user_recv(FApp app, FChannel channel)
{

	float val;
	fcap_app_get_key_f32(app, KEY_A, &val);

	printf("Server Got packet with value: %lf\n", val);

	fcap_send_all(app);
}

int main()
{
	int ret;
	FApp app;
	FChannel channel;

	/* Setup a channel */
	ret = fcap_udp_setup_channel(&udp, THIS_PORT, PEER_IP, PEER_PORT);
	if (ret < 0) {
		printf("Error: Failed to set up udp channel with code %d!\n", ret);
		exit(1);
	}

	/* Get the first instance */
	app = fcap_get_instance(0);
	if (!app) {
		printf("Error: Failed to get fcap instance!\n");
		exit(1);
	}

	/* Add the channel */
	channel = fcap_add_channel(app,
				   &udp,
				   fcap_udp_send_bytes,
				   fcap_udp_poll,
				   fcap_udp_get_bytes);
	if (!channel) {
		printf("Error: Failed to add fcap channel!\n");
		exit(1);
	}

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
