#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
    // Initialize GoogleTest Framework
    ::testing::InitGoogleTest(&argc, argv);

    // Run test cases
    return RUN_ALL_TESTS();
}
