
#include <iostream>
#include <gtest/gtest.h>

#include "chunker.h"
#include "manifest.h"
#include "fileinfo.h"


// Test FileInfo
TEST(FileInfo, LoadFile)
{
    std::string path;
    path.append("test.txt");
    FileInfo fi;

    ASSERT_EQ(fi.InitializeFile(path), true);
    std::cout<<"Get file size: "<< fi.GetFileSize() << std::endl;
 
}

// Test Manifest
TEST(ManifestTest, WriteOut)
{
    std::string file;
    file.append("manifest._mn");

    Manifest m;

    ASSERT_EQ(m.LoadManifestFile(file), true);
    ASSERT_EQ(m.WriteOutManifest(), true);
}

// Chunk a file
TEST(ChunkTest,ChunkFile)
{
    Chunker c;
    std::string test;
    test.append("test.txt");
    ASSERT_EQ(c.ChunkFile(test), true);

}

// TODO :: De-Chunk a file

int main (int argc, char* argv[])
{
    // Init gtestframework
    testing::InitGoogleTest(&argc, argv);

    // run all tests
    return RUN_ALL_TESTS();
}

