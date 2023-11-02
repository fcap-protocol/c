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
	float val1;
	fcap_app_get_key_f32(app, KEY_A, &val1);
	printf("Server Got packet with A value: %lf\n", val1);

	int64_t val2 = 0;
	int ret = fcap_app_get_key_i64(app, KEY_AA, &val2);
	if (ret < 0) {
		printf("Key AA did not have a value :( %d\n", ret);
	} else {
		printf("Server Got packet with AA value: %d\n", val2);
	}

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
		printf("Error: Failed to set up udp channel with code %d!\n",
		       ret);
		exit(1);
	}

	/* Get the first instance */
	app = fcap_init_instance(0);
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
