#include <fcap.h>
#include <fcap_udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define THIS_PORT (FCAP_PORT + 1)
#define PEER_PORT FCAP_PORT
#define PEER_IP "10.2.10.185"

FCAP_CREATE_UDP_TRANSPORT(my_udp);
FCAP_SET_TRANSPORTS(my_transports, &my_udp)

FCAP_SET_MIDDLEWARE(my_middleware)

FCAP_CREATE_APP(app, my_transports, my_middleware)

enum handler_code fcap_user_recv_req(FApp app, FEvent event, FPacket res)
{
	float val;
	fcap_app_get_key_f32(app, KEY_A, &val);

	printf("Got request!\n");

	return FCAP_CONTINUE;

}

enum handler_code fcap_user_recv_res(FApp app, FEvent event)
{
	printf("Got response!\n");
	return FCAP_CONTINUE;
}

int main()
{
	int ret;

	/* Setup a transport */
	ret = fcap_udp_setup_transport(&my_udp_priv, THIS_PORT, PEER_IP, PEER_PORT);
	if (ret < 0) {
		printf("Error: Failed to set up udp transport with code %d!\n", ret);
		exit(1);
	}

	/* Get the first instance */
	fcap_init_instance(app);

	fcap_app_add_key_f32(app, KEY_A, 12.34);

	ret = fcap_send_req(app, &my_udp);
	if (ret < 0) {
		printf("Error: Unable to send request with code %d\n", ret);
		exit(1);
	}

	printf("Sent!\n");

	exit(1);

	/* Poll everything! */
	printf("Running!\n");
	while (1) {
		ret = fcap_poll(app);
		if (ret < 0) {
			printf("Error: Failed to poll transports!");
			break;
		}
	}

	// fcap_udp_cleanup(&transport1);
	printf("Done\n");
}
