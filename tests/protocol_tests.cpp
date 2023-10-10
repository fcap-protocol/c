#include <gtest/gtest.h>
#include <cmath>

extern "C" {
#include <fcap_pkt.h>
}

TEST(FCAP_TESTS, basic_uint8)
{
	int ret;
	uint8_t recv_val;
	uint8_t sent_val = 13;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &sent_val, sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_UINT8);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, basic_uint16)
{
	int ret;
	uint16_t recv_val;
	uint16_t sent_val = 13;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_UINT16, &sent_val,
			   sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_UINT16);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, basic_int16_positive)
{
	int ret;
	int16_t recv_val;
	int16_t sent_val = std::pow(2, 15) - 1;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_INT16, &sent_val, sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_INT16);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, basic_int16_negative)
{
	int ret;
	int16_t recv_val;
	int16_t sent_val = -13;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_INT16, &sent_val, sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_INT16);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, basic_int32)
{
	int ret;
	int32_t recv_val;
	int32_t sent_val = std::pow(2, 31) - 10;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_INT32, &sent_val, sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_INT32);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, basic_int64)
{
	int ret;
	int64_t recv_val;
	int64_t sent_val = std::pow(2, 62) - 10;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_INT64, &sent_val, sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_INT64);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, basic_float)
{
	int ret;
	float recv_val;
	float sent_val = -19.28;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_FLOAT, &sent_val, sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_FLOAT);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, basic_double)
{
	int ret;
	double recv_val;
	double sent_val = -19.28;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key(pkt, KEY_A, FCAP_DOUBLE, &sent_val,
			   sizeof(sent_val));
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
	ASSERT_EQ(ret, FCAP_DOUBLE);
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, two_keys)
{
	int ret;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	uint8_t recv_val_1;
	uint8_t recv_val_2;
	uint8_t sent_val_1 = 13;
	uint8_t sent_val_2 = 42;

	/* Send value 1 */
	ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &sent_val_1,
			   sizeof(sent_val_1));
	ASSERT_EQ(ret, 0);

	/* Send value 2 */
	ret = fcap_add_key(pkt, KEY_B, FCAP_UINT8, &sent_val_2,
			   sizeof(sent_val_2));
	ASSERT_EQ(ret, 0);

	/* Get value 1 */
	ret = fcap_get_key(pkt, KEY_A, &recv_val_1, sizeof(recv_val_1));
	ASSERT_EQ(ret, FCAP_UINT8);
	ASSERT_EQ(recv_val_1, sent_val_1);

	/* Get value 2 */
	ret = fcap_get_key(pkt, KEY_B, &recv_val_2, sizeof(recv_val_2));
	ASSERT_EQ(ret, FCAP_UINT8);
	ASSERT_EQ(recv_val_2, sent_val_2);
}

TEST(FCAP_TESTS, duplicate_key)
{
	int ret;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	uint8_t recv_val_1;
	uint8_t sent_val_1 = 13;
	uint8_t sent_val_2 = 42;

	/* Send value 1 */
	ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &sent_val_1,
			   sizeof(sent_val_1));
	ASSERT_EQ(ret, 0);

	/* try set value 2 */
	ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &sent_val_2,
			   sizeof(sent_val_2));
	ASSERT_EQ(ret, -FCAP_EINVAL);

	/* Get value 1 */
	ret = fcap_get_key(pkt, KEY_A, &recv_val_1, sizeof(recv_val_1));
	ASSERT_EQ(ret, FCAP_UINT8);
	ASSERT_EQ(recv_val_1, sent_val_1);
}

TEST(FCAP_TESTS, multiple_different_keys)
{
	int ret;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	double recv_val_1;
	int32_t recv_val_2;
	double sent_val_1 = -13.31;
	int32_t sent_val_2 = 58;

	/* Send value 1 */
	ret = fcap_add_key(pkt, KEY_Z, FCAP_DOUBLE, &sent_val_1,
			   sizeof(sent_val_1));
	ASSERT_EQ(ret, 0);

	/* try set value 2 */
	ret = fcap_add_key(pkt, KEY_B, FCAP_INT32, &sent_val_2,
			   sizeof(sent_val_2));
	ASSERT_EQ(ret, 0);

	/* Get value 1 */
	ret = fcap_get_key(pkt, KEY_Z, &recv_val_1, sizeof(recv_val_1));
	ASSERT_EQ(ret, FCAP_DOUBLE);
	ASSERT_EQ(recv_val_1, sent_val_1);

	/* Get value 2 */
	ret = fcap_get_key(pkt, KEY_B, &recv_val_2, sizeof(recv_val_2));
	ASSERT_EQ(ret, FCAP_INT32);
	ASSERT_EQ(recv_val_2, sent_val_2);
}

TEST(FCAP_TESTS, basic_binary)
{
	int i;
    int ret;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	uint8_t sent_bytes[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t recv_bytes[20];

	/* Send value 1 */
	ret = fcap_add_key(pkt, KEY_Z, FCAP_BINARY, sent_bytes, 10);
	ASSERT_EQ(ret, 0);


	/* Get value 1 */
	ret = fcap_get_key(pkt, KEY_Z, recv_bytes, 20);
	ASSERT_EQ(ret, FCAP_BINARY);
    ASSERT_EQ(recv_bytes[0], 10);
    for (i = 1; i <= 10; i++)
	    ASSERT_EQ(recv_bytes[i], i);

}

TEST(FCAP_TESTS, safe_uint8)
{
	int ret;
	uint8_t recv_val;
	uint8_t sent_val = 10;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key_u8(pkt, KEY_A, sent_val);
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key_u8(pkt, KEY_A, &recv_val);
	ASSERT_EQ(ret, 0);

	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, safe_uint16)
{
	int ret;
	uint16_t recv_val;
	uint16_t sent_val = 10;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key_u16(pkt, KEY_A, sent_val);
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key_u16(pkt, KEY_A, &recv_val);
	ASSERT_EQ(ret, 0);
	
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, safe_int16)
{
	int ret;
	int16_t recv_val;
	int16_t sent_val = -10;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key_i16(pkt, KEY_A, sent_val);
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key_i16(pkt, KEY_A, &recv_val);
	ASSERT_EQ(ret, 0);
	
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, safe_int32)
{
	int ret;
	int32_t recv_val;
	int32_t sent_val = -10;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key_i32(pkt, KEY_A, sent_val);
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key_i32(pkt, KEY_A, &recv_val);
	ASSERT_EQ(ret, 0);
	
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, safe_int64)
{
	int ret;
	int64_t recv_val;
	int64_t sent_val = -10;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key_i64(pkt, KEY_A, sent_val);
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key_i64(pkt, KEY_A, &recv_val);
	ASSERT_EQ(ret, 0);
	
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, safe_float)
{
	int ret;
	float recv_val;
	float sent_val = 0.1234567;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key_f32(pkt, KEY_A, sent_val);
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key_f32(pkt, KEY_A, &recv_val);
	ASSERT_EQ(ret, 0);
	
	ASSERT_EQ(recv_val, sent_val);
}

TEST(FCAP_TESTS, safe_double)
{
	int ret;
	double recv_val;
	double sent_val = -0.12345671234567;
	fcap_packet_t packet;
	FPacket pkt = &packet;
	fcap_init_packet(pkt);

	ret = fcap_add_key_d64(pkt, KEY_A, sent_val);
	ASSERT_EQ(ret, 0);

	ret = fcap_get_key_d64(pkt, KEY_A, &recv_val);
	ASSERT_EQ(ret, 0);
	
	ASSERT_EQ(recv_val, sent_val);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
