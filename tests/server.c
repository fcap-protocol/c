#include <fcap.h>
#include <fcap_udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define THIS_PORT FCAP_PORT
#define PEER_PORT (FCAP_PORT + 1)
#define PEER_IP "127.0.0.1"

FCAP_CREATE_UDP_TRANSPORT(my_udp);
FCAP_SET_TRANSPORTS(my_transports, &my_udp)

FCAP_SET_MIDDLEWARE(my_middleware)

FCAP_CREATE_APP(app, my_transports, my_middleware)

enum handler_code fcap_user_recv_req(FApp app, FEvent event, FPacket res)
{
	float val;
	fcap_app_get_key_f32(app, KEY_A, &val);

	printf("Got request!\n");

	fcap_debug_packet(event->pkt);
	printf("\n");

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
	ret = fcap_udp_setup_transport(
		&my_udp_priv, THIS_PORT, PEER_IP, PEER_PORT);
	if (ret < 0) {
		printf("Error: Failed to set up udp transport with code %d!\n",
		       ret);
		exit(1);
	}

	/* Get the first instance */
	fcap_init_instance(app);

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
