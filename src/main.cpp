
#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "utils.h"
#include "chunker.h"
#include "manifest.h"
#include "filemanager.h"
#include "crypto.h"



TEST(CRYPTO, Keys)
{
    Crypto crp;
    Credentials cred = crp.GenerateCredentials();
    std::cout << "KEY : " <<std::endl;
    std::cout << cred.key << "\n";
    std::cout << "IV : " << cred.iv << "\n";


}


TEST(UTILS, StringSplitter)
{
    std::vector<std::string> out;
    std::string s;
    s.append("This\tis\tthe\tstring\tI'm\tsplitting\n");

    utils::SplitString(s, '\t', out);
    ASSERT_EQ(out.size(), 6);
}

// Test FileManager
TEST(FileManager, IndexFile)
{
    std::string manifestpath;
    manifestpath.append("./data/manifest._mn");
    std::string path;
    path.append("./data/test.pdf");
 
    FileManager fm(manifestpath);
    ASSERT_EQ(fm.FileExists(path), true);
    //ASSERT_EQ(mf.CreateEmptyManifest(), true);
    ASSERT_EQ(fm.StartupFileManager(), true);
    ASSERT_EQ(fm.IndexFile(path), true);
    ASSERT_EQ(fm.ShutdownFileManager(), true);

    std::string filename;
    filename.append("test.pdf");
    ASSERT_EQ(fm.ConstructFile(filename), true);

    try
    {
        std::string output;
        output.append("testcomp");
        fm.CompressFile(path, output, 1); 
        std::string uncomp;
        uncomp.append("uncompressed");
        fm.DecompressFile(output, uncomp);

    }
    catch (std::exception& e)
    {
        std::cout << "EXCEPTION : " << e.what() << std::endl;
    } 


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

