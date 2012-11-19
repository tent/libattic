
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



#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>

void function_pt(void *ptr, size_t size, size_t nmemb, void *stream){

    
 //       printf("%d", atoi(ptr));
}
struct tstring {
    char *ptr;
    size_t len;
};
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct tstring *s)
{
    size_t new_len = s->len + size*nmemb;

    // DELETE THIS right now im leaking
    //s->ptr = realloc(s->ptr, new_len+1);

    s->ptr = new char[new_len+1];

    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

void init_string(struct tstring *s) {
    s->len = 0;
    s->ptr = new char[(s->len+1)];
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

TEST(CURL, get)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    curl = curl_easy_init();

    tstring s;
    init_string(&s);
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://manuel.tent.is/");
        //curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            std::cout<<"ERRR"<<std::endl;

        std::string a;
        a.append(s.ptr, s.len);
        std::cout<< " REQUEST : " <<a << std::endl;
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

}

extern "C"
{
    #include "crypto_scrypt.h"
    int crypto_scrypt(const uint8_t *, size_t, const uint8_t *, size_t, uint64_t,
            uint32_t, uint32_t, uint8_t *, size_t);
}

TEST(SCRYPT, ENCRYPT)
{
    uint8_t salt[32]; // 16 <- do 16, 64 or 128

    uint8_t* password;
    size_t plen;

    uint64_t N = 16384;
    uint32_t r = 8;
    uint32_t p = 1;

    uint8_t dk[64];

    //password = new uint8_t[256];
    //memcpy(password, "thisismypassword", 16);
    //plen = 16;

    // Recommended numbers
    // N=16384, r=8, p=1
    std::cout<< " PASS : " << password << std::endl;
    std::cout << crypto_scrypt((uint8_t*)"pw", 2, (uint8_t*)"salt", 4, N, r, p, dk, 64) << std::endl;
    std::cout << "DK " << dk << std::endl;
    // This produces they key to be used to encrypt the other keys.
}

/*
TEST(COMPRESS, ENCRYPT)
{
    // Compress
    Compressor cmp;
    std::string path;
    path.append("./data/test.pdf");

    std::string compressedout;
    compressedout.append("./output/testcomp");
    ASSERT_EQ(cmp.CompressFile(path, compressedout, 1), ret::A_OK);

    unsigned int size = utils::CheckFileSize(compressedout);

    std::cout<<"FILE SIZE : "<< size << std::endl;

    // Encrypt
    Crypto crp;
    Credentials cred = crp.GenerateCredentials();

    std::string encrypt;
    encrypt.append("./output/cryp");
    // encrypt file
    ASSERT_EQ(crp.EncryptFile(compressedout, encrypt, cred), ret::A_OK);
   

    Chunker chnk;
    FileInfo* fi = new FileInfo();

    // chunk file
    std::string chunkDir;
    chunkDir.append("./output");

    ASSERT_EQ(chnk.ChunkFile(fi, encrypt, chunkDir), ret::A_OK);


    // Now undo the process

    // Unchunk
    std::string unchunk;
    unchunk.append("./output/unchunked");
    chnk.DeChunkFile(fi, unchunk, chunkDir);

    // Decrypt
    std::string uncryp;
    uncryp.append("./output/uncryp");
    // decrypt file
    ASSERT_EQ(crp.DecryptFile(unchunk, uncryp, cred), ret::A_OK);

    // Decompress
    std::string uncomp;
    uncomp.append("./output/uncompressed");
    ASSERT_EQ(cmp.DecompressFile(uncryp, uncomp), ret::A_OK);

}


*/

/*
TEST(CHUNKER, Chunking)
{

    Chunker chnk;
    FileInfo* fi = new FileInfo();

    std::string path;
    path.append("./data/test.pdf");

    std::string chunkDir;
    chunkDir.append("./output");
 

    ASSERT_EQ(chnk.ChunkFile(fi, path, chunkDir), ret::A_OK);

    std::string outBound;
    outBound.append("./output/dechunked");
    ASSERT_EQ(chnk.DeChunkFile(fi, outBound, chunkDir), ret::A_OK); 
}

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

    std::string workingdir;
    workingdir.append("./output");
 
    FileManager fm(manifestpath, workingdir);

    ASSERT_EQ(fm.FileExists(path), true);
    //ASSERT_EQ(mf.CreateEmptyManifest(), true);
    ASSERT_EQ(fm.StartupFileManager(), true);

    ret::eCode status = fm.IndexFile(path);
    ASSERT_EQ(status, ret::A_OK);

    std::cout<< "STATUS : " << status << std::endl;
    ASSERT_EQ(fm.ShutdownFileManager(), true);


 //   std::string filename;
//    filename.append("test.pdf");
//    ASSERT_EQ(fm.ConstructFile(filename), true);
}


*/

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

