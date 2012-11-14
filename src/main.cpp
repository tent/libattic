
#include <iostream>
#include <gtest/gtest.h>

#include "utils.h"
#include "chunker.h"
#include "manifest.h"
#include "filemanager.h"


// Test FileManager
TEST(FileManager, IndexFile)
{
    std::string manifestpath;
    manifestpath.append("./manifest._mn");
    std::string path;
    path.append("test.txt");
 
    FileManager fm(manifestpath);
    //ASSERT_EQ(fm.FileExists(path), true);
    //ASSERT_EQ(mf.CreateEmptyManifest(), true);
    ASSERT_EQ(fm.StartupFileManager(), true);
    ASSERT_EQ(fm.IndexFile(path), true);
    ASSERT_EQ(fm.ShutdownFileManager(), true);

 

}
/*
// Test FileInfo
TEST(FileInfo, LoadFile)
{
    std::string path;
    path.append("test.txt");
    FileInfo fi;

    ASSERT_EQ(fi.InitializeFile(path), true);
    std::cout<<"Get file size: "<< fi.GetFileSize() << std::endl;
 
}
*/

// TODO :: De-Chunk a file

int main (int argc, char* argv[])
{
    // Init gtestframework
    testing::InitGoogleTest(&argc, argv);

    // run all tests
    return RUN_ALL_TESTS();
}

