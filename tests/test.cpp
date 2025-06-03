// Anisimov Vasiliy st129629@student.spbu.ru
// Laboratory Work 2

#include <gtest/gtest.h>
#include "jump_list.h"

TEST(jump_list, Example) {
    EXPECT_EQ(1, 1);
}

// Main function for running tests
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
