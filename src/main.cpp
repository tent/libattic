
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
#include "connectionmanager.h"
#include "urlvalues.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>

#include <json/json.h>

#include "tentapp.h"

#include "libattic.h"

#include "url.h"


TEST(MULT, GET)
{
    std::string szAppPath("./app");
    int status = LoadAppFromFile(szAppPath.c_str());

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    // Load Access Token
    status = LoadAccessToken("./at");

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    status = GetFile("https://manuel.tent.is/tent/posts/mpb17n/attachments/thisthing.lst", "");

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);
}
/*
 */
/*
TEST(MULTI, PART)
{
    std::string szAppPath("./app");
    int status = LoadAppFromFile(szAppPath.c_str());

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    // Load Access Token
    status = LoadAccessToken("./at");

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);



    status = PostFile("https://manuel.tent.is/tent/posts", "./testfile");

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

}
/*
*/


/*  
TEST(MAC, SIGNING)
{
    //"http://example.com/resource/1?b=1&a=2", "POST", "hmac-sha-256", "Xt51rtHY5F+jxKXMCoiKgXa3geofWW/7RANCXB1yu08="
    
    Url test("http://example.com/resource/1?b=1&a=2");
    std::string out;
    ConnectionManager::GetInstance()->BuildAuthHeader(std::string("http://example.com/resource/1?b=1&a=2"), std::string("POST"), std::string(""), std::string("489dks293j39"), out);


    std::cout<< " AUTH HEADER : " << out << std::endl;
    

}

TEST(URL, CUSTOM)
{
    Url test("http://www.something.com:8080/this/is/a/path.swf?something=whatever");
    
    std::cout << test.GetUrl() << std::endl;
    std::cout << test.GetScheme() << std::endl;
    std::cout << test.GetHost() << std::endl;
    std::cout << test.GetPath() << std::endl;
    std::cout << test.GetPort() << std::endl;
    std::cout << test.GetQuery() << std::endl;

}
/*
*/
/*
TEST(LIBATTIC, CODEUSAGE)
{

    std::string szAppPath("./app");
    int status = LoadAppFromFile(szAppPath.c_str());

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    status = RequestUserAuthorizationDetails("https://manuel.tent.is/tent/", "6c61a78bf2de1b25c356b0b0fddebf19");

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

}
/*  
*/
/*
TEST(LIBATTIC, STARTAPPINST)
{

    char* p[] = { "https://manuel.tent.is" };
    char* s[] = { "read_posts", 
                  "write_posts",
                  "import_posts",
                  "read_profile",
                  "write_profile",
                  "read_followers",
                  "write_followers",
                  "read_followings",
                  "write_followings",
                  "read_groups",
                  "write_groups",
                  "read_permissions",
                  "write_permissions",
                  "read_apps",
                  "write_apps",
                  "follow_ui",
                  "read_secrets",
                  "write_secrets"};


    int status = StartupAppInstance("libattic", "This is an app", "www.tent.is", "", p,1, s, 18);
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    status = RegisterApp("https://manuel.tent.is/tent/apps");
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    status = RequestAppAuthorizationURL("https://manuel.tent.is/tent/");
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    std::cout<<"URL : " << GetAuthorizationURL() << std::endl;

    std::string szAppPath("./app");
    status = SaveAppToFile(szAppPath.c_str());

    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    status = ShutdownAppInstance();
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);


}
/*
*/
/*
TEST(CONNECTIONMANAGER, POST)
{
    std::string postpath("https://manuel.tent.is/tent/apps");

    TentApp app;
    app.SetAppName(std::string("COOL APP"));
    app.SetAppDescription(std::string("this is a cool app"));
    app.SetAppURL(std::string("http://manuel.tent.is"));
    app.SetScope(std::string("read_posts"));
    app.SetScope(std::string("write_posts"));
    app.SetRedirectURI(std::string("https://manuel.tent.is"));

    std::string serialized;

    JsonSerializer::SerializeObject(&app, serialized);

    ConnectionManager* pCm = ConnectionManager::GetInstance();

    std::string responseOut;
    pCm->HttpPost(postpath, serialized, responseOut, true);

    std::cout<< " RESPONSE : " << responseOut << std::endl;
    pCm->Shutdown();

    TentApp app2;
    JsonSerializer::DeserializeObject(&app2, responseOut);

    std::string s2;
    JsonSerializer::SerializeObject(&app2, s2);
    std::cout<< " S2 : \n" << s2 << std::endl;
}
*/
/*
TEST(CURL, POST)
{

    std::string postpath("https://manuel.tent.is/tent/apps");
    std::string body;

    ConnectionManager* pCm = ConnectionManager::GetInstance();

    TentApp app;
    app.SetAppName(std::string("COOL APP"));
    app.SetAppDescription(std::string("this is a cool app"));
    app.SetScope(std::string("everywhere duh"));

    std::string serialized;
    JsonSerializer::SerializeObject(&app, serialized);

    std::cout<<"SERIALIZED : " << serialized << std::endl;

    pCm->HttpPost(postpath, serialized);
    pCm->Shutdown();
}
*/
    /*
    TEST(JSON, SERIALIZEAPP)
    {
    TentApp app;
    app.SetAppName(std::string("COOL APP"));
    app.SetAppDescription(std::string("this is a cool app"));
    app.SetScope(std::string("everywhere duh"));

    std::string serialized;
    JsonSerializer::SerializeObject(&app, serialized);

    std::cout<<"SERIALIZED : " << serialized << std::endl;

    TentApp app2;

    JsonSerializer::DeserializeObject(&app2, serialized);

    std::cout<<" APP2 NAME : " << app2.GetAppName() << std::endl;

    }

    TEST(URLVALUES, ADDSERIALIZE)
    {
    UrlValues val;
    std::string key = "client_id";
    std::string value = "t1jrsh";

    val.AddValue(key, value);


    key.clear();
    key.append("redirect_uri");
    value.clear();
    value.append("http://app.example.com/tent/callback");

    val.AddValue(key, value);

    key.clear();
    key.append("scope");
    value.clear();
    value.append("read_posts");
    val.AddValue(key, value);
    value.clear();
    value.append("read_profile");

    val.AddValue(key, value);

    std::cout<<" SERIALIZE : " << val.SerializeToString() << std::endl;
    }

    TEST(CONNECTIONMANAGER, CONNECT)
    {

    ConnectionManager* pCm = ConnectionManager::GetInstance();

    std::string url;
    url.append("https://manuel.tent.is/tent/followings");
    std::string result =  pCm->HttpGet(url);
    std::cout<<" RESULTS : " << result << std::endl;

    pCm->Shutdown();

    }
    */
    /*


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

    delete s->ptr;
    s->ptr = 0;
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
        curl_easy_setopt(curl, CURLOPT_URL, "https://manuel.tent.is/tent/followings");
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
    */
    /*
     */
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

