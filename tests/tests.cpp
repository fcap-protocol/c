#include <gtest/gtest.h>

extern "C" {
#include <fcap.h>
}

TEST(FCAP_TESTS, basic)
{
	int ret;
	FPacket pkt = fcap_init_packet();
	uint8_t *bytes = (uint8_t *)pkt;

	uint8_t sent_val = 13;
	ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &sent_val, sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	uint8_t recv_val;
	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_UINT8);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, two_keys)
{
	int ret;
	FPacket pkt = fcap_init_packet();
	uint8_t *bytes = (uint8_t *)pkt;

	/* Send value 1 */
	uint8_t sent_val_1 = 13;
	ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &sent_val_1,
			   sizeof(sent_val_1));
	ASSERT_EQ(ret, 0);

	/* Send value 2 */
	uint8_t sent_val_2 = 42;
	ret = fcap_add_key(pkt, KEY_B, FCAP_UINT8, &sent_val_2,
			   sizeof(sent_val_2));
	ASSERT_EQ(ret, 0);

	/* Get value 1*/
	uint8_t recv_val_1;
	ret = fcap_get_key(pkt, KEY_A, &recv_val_1, sizeof(recv_val_1));
	ASSERT_EQ(ret, FCAP_UINT8);
	ASSERT_EQ(recv_val_1, sent_val_1);

	/* Get value 2 */
	uint8_t recv_val_2;
	ret = fcap_get_key(pkt, KEY_B, &recv_val_2, sizeof(recv_val_2));
	ASSERT_EQ(ret, FCAP_UINT8);
	ASSERT_EQ(recv_val_2, sent_val_2);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
