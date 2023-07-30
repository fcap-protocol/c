#include <gtest/gtest.h>

extern "C" {
#include <fcap.h>
}

TEST(MultiplyTests, basic) {
    const auto expected = 50;
    const auto actual = multiply(5, 10);
    ASSERT_EQ(expected, actual);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}