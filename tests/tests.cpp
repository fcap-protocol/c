#include <gtest/gtest.h>

extern "C" {
#include <fcap.h>
}


TEST(FCAP_TESTS, basic) {
    int ret;
    FPacket pkt = fcap_init_packet();
    uint8_t *bytes = (uint8_t *) pkt;

    uint8_t sent_val = 13; 
    ret = fcap_add_key(pkt, KEY_A, FCAP_UINT8, &sent_val, sizeof(sent_val));
    ASSERT_EQ(ret, 0);

    uint8_t recv_val;
    ret = fcap_get_key(pkt, KEY_A, &recv_val, sizeof(recv_val));
    ASSERT_EQ(ret, FCAP_UINT8);
    ASSERT_EQ(recv_val, sent_val);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
