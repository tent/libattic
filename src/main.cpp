
#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "utils.h"
#include "chunker.h"
#include "manifest.h"
#include "filemanager.h"
#include "crypto.h"
#include "compressor.h"
#include "errorcodes.h"


TEST(CRYPTO, Keys)
{
    Crypto crp;
    Credentials cred = crp.GenerateCredentials();

    std::string path;
    path.append("./data/test.pdf");
 
    std::string out;
    out.append("./output/cryp");
    // encrypt file
    ASSERT_EQ(crp.EncryptFile(path, out, cred), ret::A_OK);

    std::string uncryp;
    uncryp.append("./output/uncryp");
    // decrypt file
    ASSERT_EQ(crp.DecryptFile(out, uncryp, cred), ret::A_OK);
}

TEST(COMPRESSOR, Compress)
{
    Compressor cmp;
    std::string path;
    path.append("./data/test.pdf");

    std::string output;
    output.append("./output/testcomp");
    ASSERT_EQ(cmp.CompressFile(path, output, 1), ret::A_OK);

}

TEST(COMPRESSOR, Decompress)
{
    Compressor cmp;
    std::string compfile;
    compfile.append("./output/testcomp");

    std::string uncomp;
    uncomp.append("./output/uncompressed");
    ASSERT_EQ(cmp.DecompressFile(compfile, uncomp), ret::A_OK);
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

