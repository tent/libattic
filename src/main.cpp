
#include "chunker.h"
#include <gtest/gtest.h>

TEST(ChunkTest,ChunkFile) {
    Chunker c;
    ASSERT_EQ(c.ChunkFile("test.txt"),true);
}

int main (int argc, char* argv[])
{
    // Init gtestframework
    testing::InitGoogleTest(&argc, argv);

    // run all tests
    return RUN_ALL_TESTS();
}

