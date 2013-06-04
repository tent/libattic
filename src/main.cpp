#include <iostream>
#include <vector>
#include <deque>

#include <gtest/gtest.h>
#include <cbase64.h>
 
#include "utils.h"
#include "manifest.h"
#include "filemanager.h"
#include "crypto.h"
#include "errorcodes.h"
#include "urlparams.h"

#include <stdio.h>
#include <stdlib.h>

#include <json/json.h>

#include "tentapp.h"

#include "libattic.h"
#include "url.h"
#include "threading.h"
#include "rollsum.h"

#include "netlib.h"
#include "compression.h"

#include "filesystem.h"
#include "folder.h"
#include "folderpost.h"

#include "rollsum.h"
#include "constants.h"
#include "httpheader.h"

#include "event.h"
#include "threadworker.h"
#include "filequeue.h"
#include "callbackhandler.h"
#include "atticclient.h"
#include "chunkbuffer.h"


// Temporary test, hooked up to localhost tent server
/*
TEST(ATTIC_SERVICE, START_STOP)
{
    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", "http://localhost:8080");
    int status = InitLibAttic();
    //status = EnterPassphrase("password");
    status = EnterPassphrase("asdf");
    std::cout<<" Enter passphrase status : " << status << std::endl;
    ASSERT_EQ(status, attic::ret::A_OK);

   sleep(10);

    ShutdownLibAttic(NULL);
}
*/

TEST(PROCESS, COMPRESS_ENCRYPT_DECRYPT_COMPRESS)
{
    std::string test("This is a test string, of some sort of data, it's pretty great");

    // Compress
    std::string compressed;
    attic::compress::CompressString(test, compressed);
    // Encrypt
    attic::Credentials cred = attic::crypto::GenerateCredentials();
    std::string encrypted;
    attic::crypto::EncryptStringCFB(compressed, cred, encrypted);
    // Decrypt
    std::string decrypted;
    attic::crypto::DecryptStringCFB(encrypted, cred, decrypted);
    // Decompress
    std::string decompressed;
    attic::compress::DecompressString(decrypted, decompressed);

    ASSERT_EQ(test, decompressed);

}

TEST(COMPRESS, COMPRESSSTRING)
{
    std::string in("this is my test string it is a pretty decent test string");
    std::string out;

    attic::compress::CompressString(in, out);

    std::string decomp;
    attic::compress::DecompressString(out, decomp);

    ASSERT_EQ(in, decomp);
}

TEST(NETLIB, EXTRACTHOSTANDPATH)
{
    std::string url = "https://manuel.tent.is/tent/posts";
    std::string protocol, host, path;

    attic::netlib::ExtractHostAndPath(url, protocol, host, path);
    ASSERT_EQ(host, std::string("manuel.tent.is"));
    ASSERT_EQ(path, std::string("/tent/posts"));
}

TEST(CREDENTIALS, ISEMPTY)
{
    attic::Credentials cred;

    ASSERT_EQ(cred.key_empty(), true);
    ASSERT_EQ(cred.iv_empty(), true);

    cred = attic::crypto::GenerateCredentials();

    ASSERT_EQ(cred.key_empty(), false);
    ASSERT_EQ(cred.iv_empty(), false);
}

// run credentials through the filepost upload / download process
TEST(CREDENTIALS, GENERATE) {
    std::string master_key("qwertyuiopasdfghjklzxcvbnmasdfgh");
    for(int i=0; i<100; i++) {
        attic::Credentials local_cred;
        // Generate file credentials
        attic::crypto::GenerateCredentials(local_cred);
        // Encry file key
        attic::Credentials tcred;   // transiet cred
        tcred.set_key(master_key);
        tcred.set_iv(local_cred.iv());

        std::string local_key = local_cred.key();
        std::string encrypted_key;
        attic::crypto::EncryptStringGCM(local_key, tcred, encrypted_key);

        attic::Credentials enc_cred; // Final encrytred file key
        enc_cred.set_iv(local_cred.iv());

        // encode key and iv
        std::string b64_key, b64_iv;
        attic::crypto::Base64EncodeString(encrypted_key, b64_key);
        attic::crypto::Base64EncodeString(enc_cred.iv(), b64_iv);

        // reverse the process
        std::string key, iv;
        attic::crypto::Base64DecodeString(b64_key, key);
        attic::crypto::Base64DecodeString(b64_iv, iv);

        // create transient cred
        attic::Credentials dtcred;
        dtcred.set_key(master_key);
        dtcred.set_iv(iv);

        std::string decrypted_key;
        attic::crypto::DecryptStringGCM(key, dtcred, decrypted_key);

        /*
        std::cout<<" key attempt : " << i << " ";
        if(local_cred.key() != decrypted_key )
            std::cout<<" failed " << std::endl;
        else
            std::cout<<" passed " << std::endl;
            */
        ASSERT_EQ(strcmp(local_cred.key().c_str(), decrypted_key.c_str()), 0);
    }
}

TEST(CRYPTO, BASE32)
{
    std::string teststring("this is my test string, that I'm going to base32 encode");
    std::string encoded;
    attic::crypto::Base32EncodeString(teststring, encoded);

    std::string decoded;
    attic::crypto::Base32DecodeString(encoded, decoded);
    ASSERT_EQ(teststring, decoded);
}

TEST(CRYPTO, BASE64)
{
    std::string teststring("this is my test string, that I'm going to base64 encode");
    std::string encoded;
    attic::crypto::Base64EncodeString(teststring, encoded);
    std::string decoded;
    attic::crypto::Base64DecodeString(encoded, decoded);
    ASSERT_EQ(teststring, decoded);
}

TEST(CRYPTO, HMAC)
{
    int status = attic::ret::A_OK;
    attic::Credentials cred = attic::crypto::GenerateCredentials();

    std::string plaintext("this is my plain text");

    std::string macout;
    status = attic::crypto::GenerateHMACForString( plaintext, cred, macout);
    ASSERT_EQ(status, attic::ret::A_OK);

    status = attic::crypto::VerifyHMACForString( plaintext, cred, macout);
    ASSERT_EQ(status, attic::ret::A_OK);
}

TEST(CRYPTO, ENCRYPTIONCFB)
{
    attic::Credentials cred = attic::crypto::GenerateCredentials();

    std::string plaintext("this is my plain text");

    std::string cyphertext;
    attic::crypto::EncryptStringCFB(plaintext, cred, cyphertext);

    std::string decryptedtext;
    attic::crypto::DecryptStringCFB(cyphertext, cred, decryptedtext);

    ASSERT_EQ(plaintext, decryptedtext);
}

TEST(CRYPTO, CREDENCRYPTIONGCM)
{
    attic::Credentials masterkey;

    std::string phrase("this is a test");
    std::string iv;
    attic::crypto::GenerateSalt(iv);

    // Genterate key from passphrase
    int status = attic::crypto::GenerateKeyFromPassphrase(phrase,
                                                   iv,
                                                   masterkey);
    ASSERT_EQ(status, 0);

    attic::Credentials cred; // Credentials to encrypt
    cred = attic::crypto::GenerateCredentials();

    std::string key = cred.key();

    attic::Credentials intercred; // credentials used to encrypt file key
                           // master key
                           // file specific iv

    std::string mk = masterkey.key();
    std::string fileiv = cred.iv();

    intercred.set_key(mk);
    intercred.set_iv(fileiv);

    std::string enckey;
    status = attic::crypto::EncryptStringGCM(key, intercred, enckey);
    ASSERT_EQ(status, 0);

    // Generate key again for good measure
    attic::Credentials mkcopy;
    status = attic::crypto::GenerateKeyFromPassphrase(phrase,
                                               iv,
                                               mkcopy);
    ASSERT_EQ(status, 0);

    attic::Credentials intercred1;
    std::string mk1 = mkcopy.key();

    intercred1.set_key(mk1);
    intercred1.set_iv(fileiv);

    std::string deckey;
    status = attic::crypto::DecryptStringGCM(enckey, intercred1, deckey);

    ASSERT_EQ(status, 0);
    ASSERT_EQ(key, deckey);
}

/*
TEST(CRYPTO, FILEHASH) {

   std::string filepath = "./data/m4.mp4";
   std::string hash_out;
   attic::crypto::GenerateFileHash(filepath, hash_out);
   std::cout<<" FILE HASH : " << hash_out << std::endl;
}
*/

TEST(SCRYPT, ENTER_PASSPHRASE)
{
    attic::Credentials cred, cred1;

    std::string pw("password");
    std::string iv;
    attic::crypto::GenerateSalt(iv); 

    int status = attic::crypto::GenerateKeyFromPassphrase( pw,
                                               iv,
                                               cred);
    
    ASSERT_EQ(status, 0);
    status = attic::crypto::GenerateKeyFromPassphrase( pw ,
                                  iv,
                                  cred1);

    ASSERT_EQ(status, 0);
    ASSERT_EQ(cred.key(), cred1.key());
    ASSERT_EQ(cred.iv(), cred1.iv());
}

TEST(SCRYPT, ENCODE)
{
    std::string input("thisistestinput");
    std::string iv;

    attic::crypto::GenerateSalt(iv); 

    std::string out, out1;
    attic::crypto::ScryptEncode(input, iv, out, CryptoPP::AES::MAX_KEYLENGTH);
    attic::crypto::ScryptEncode(input, iv, out1, CryptoPP::AES::MAX_KEYLENGTH);

    int res =  strcmp(out.c_str(), out1.c_str());
    ASSERT_EQ(res, 0);
}

TEST(FILESYSTEM, RELATIVETO)
{
    std::string relative;
    attic::fs::MakePathRelative("foo/bar", "this/foo/test/something/what.txt", relative);
    ASSERT_EQ(relative, std::string("../../this/foo/test/something/what.txt"));
}

TEST(FILESYSTEM, EXTRACT_DOUBLE_QUOTES) {
    std::string wrong("//this//is//totally//the//wrong//path//right.md");
    std::string right("/this/is/totally/the/wrong/path/right.md");
    attic::fs::ErrorCheckPathDoubleQuotes(wrong);
    ASSERT_EQ(wrong, right);
}

/*
TEST(FILESYSTEM, SUBDIRECTORIES)
{
    std::string root("data");
    std::string test("data/test/folder/this/is/mine/what.pdf");

    std::vector<std::string> list;
    attic::utils::ExtractSubPaths(root, test, list);

    std::vector<std::string>::iterator itr = list.begin();
    for(;itr!=list.end(); itr++) {
        std::cout<< *itr << std::endl;
    }
}
*/

TEST(AUTHCODE, GENERATE)
{
    std::string authcode;
    attic::utils::GenerateRandomString(authcode, 32);
    std::string encoded;
    attic::crypto::Base32EncodeString(authcode, encoded);
    std::string decoded;
    attic::crypto::Base32DecodeString(encoded, decoded);
    ASSERT_EQ(decoded, authcode);

}

/*
TEST(MANIFEST, INSERTFILEINFO)
{
    attic::FileInfo fi( "somefile.pdf",
                 "foo/bar/somefile.pdf",
                 "kjsadkfj-3412",
                 "ASDFf=342jf#-_");

    Manifest mf;
    std::string dir("");
    mf.SetDirectory(dir);
    int status = mf.Initialize();
    ASSERT_EQ(status, attic::ret::A_OK);

    ASSERT_EQ(mf.Insertattic::FileInfo(fi), true);

    mf.Shutdown();
}

TEST(MANIFEST, INSERTFILEPOSTID)
{
    Manifest mf;
    std::string dir("");
    mf.SetDirectory(dir);
    int status = mf.Initialize();
    ASSERT_EQ(status, attic::ret::A_OK);

    std::string filepath("foo/bar/somefile.pdf");
    std::string postid("ksjAF934-dsaf#_=");
    ASSERT_EQ(mf.InsertFilePostID(filepath, postid), true);

    mf.Shutdown();
}

TEST(MANIFEST, REMOVEFILEINFO)
{
    Manifest mf;
    std::string dir("");
    mf.SetDirectory(dir);
    int status = mf.Initialize();
    ASSERT_EQ(status, attic::ret::A_OK);

    std::string filepath("foo/bar/somefile.pdf");
    ASSERT_EQ(mf.Removeattic::FileInfo(filepath), true);

    mf.Shutdown();
}
*/
TEST(CHUNKINFO, SERIALIZATION) 
{
    attic::ChunkInfo ci("name", "supersummmmmmm");
    ci.set_ciphertext_mac("aksdjfkasdfCIPHER");
    ci.set_plaintext_mac("THIS IS MY PLAIN TEXT MAC MOFO ");
    ci.set_iv("IVVV");

    std::string output;
    attic::jsn::SerializeObject(&ci, output);

    attic::ChunkInfo ci2;
    attic::jsn::DeserializeObject(&ci2, output);
    

    std::string output2;
    attic::jsn::SerializeObject(&ci2, output2);

    ASSERT_EQ(output, output2);
}

/*
TEST(PARAMS, ENCODE) 
{
    UrlParams params;                                                                  
    params.AddValue(std::string("post_types"), std::string(attic::cnst::g_szFolderPostType)); 
    params.AddValue(std::string("limit"), std::string("200"));                     

    std::string enc;                           
    params.SerializeAndEncodeToString(enc);  

    std::string consume;
    consume = "Cache-Control: max-age=0, private, must-revalidate\r\n"
              "Content-Type: application/vnd.tent.v0+json\r\n"
              "Date: Wed, 06 Mar 2013 15:51:58 GMT\r\n"
              "Etag: \"fc96cd64868268c1940a382bdd4eae9f\"\r\n"
              "Link: </posts?limit=1&post_types=https%3A%2F%2Fcupcake.io%2Ftypes%2Fpost%2Fattic-folder%2Fv0.1.0&since_id=4WAPeWUmBoM1Stq02BVMfA&since_id_entity=https%3A%2F%2Fpolar-springs-6638.herokuapp.com>; rel=\"prev\", </posts?limit=1&post_types=https%3A%2F%2Fcupcake.io%2Ftypes%2Fpost%2Fattic-folder%2Fv0.1.0&before_id=4WAPeWUmBoM1Stq02BVMfA&before_id_entity=https%3A%2F%2Fpolar-springs-6638.herokuapp.com>; rel=\"next\"\r\n";

    HttpHeader head;
    head.ParseString(consume);
    std::string out;
    head.ReturnAsString(out);

    head["TEST"] = "testing operator overloading, do i still remember?";

    std::string random;
    attic::crypto::GenerateRandomString(random, 4);
    std::cout<<" RANDOM STRING : " << random << std::endl;
}
*/

/*
TEST(PARAMS, DECODE) {
    std::string encoded = "?before_post=http%3A%2F%2Fbb216a47d970.alpha.attic.is+A3QNgRslTgFL7izr76eXiQ&limit=2&since_time=0";
    std::cout << " encoded :" << encoded << std::endl;
    attic::UrlParams params;
    params.DeserializeEncodedString(encoded);
    std::cout<<" decoded : " << params.asString() << std::endl;
    std::string encode_two;
    params.SerializeAndEncodeToString(encode_two);
    std::cout<<" encoded again : " << encode_two << std::endl;


    attic::UrlParams params_two;
    params_two.AddValue("something", "id.something.text + blah=20");
    std::cout<< " test encode : " << params_two.asString() << std::endl;
    std::string test_encode;
    params_two.SerializeAndEncodeToString(test_encode);
    std::cout<< " test encoded : " << test_encode << std::endl;

}
*/

/*
TEST(FILESYSTEM, SCAN)
{
    std::vector<std::string> paths;
    attic::fs::ScanDirectory("./data", paths);
    for(unsigned int i=0; i<paths.size(); i++) {
        std::cout<<paths[i]<<std::endl;
    }
}
*/
/*
TEST(SOCKET, CONNECTION) {
    boost::asio::io_service io_service;
    attic::Connection con(&io_service);
    try {
        con.Initialize("https://www.pcwebshop.co.uk/");
    }
    catch(std::exception& e) {
        std::cout<<e.what()<< std::endl;
    }
}
*/

/*
TEST(CHUNKBUFFER, TEST) {
    attic::ChunkBuffer cb;
    if(cb.OpenFile("./data/cfs.mp4")) { 
        std::ofstream ofs;
        ofs.open("test.mp4", std::ios::out | std::ios::binary);

        std::string chunk;
        while(cb.ReadChunk(chunk)) {
            std::cout<<" got a chunk " << std::endl;
            std::cout<<" chunk size : " << chunk.size() << std::endl;
            ofs.write(chunk.c_str(), chunk.size());
            chunk.clear();
        }
        ofs.close();
    }
}
*/

/*
#include "taskcontext.h"

TEST(SIZE, TEST) {
    std::cout<<" SIZE TESTS " << std::endl;
    attic::TaskContext tc;                                       
    tc.set_value("file_type", "file");                    
    tc.set_value("original_filepath", std::string("this is a test")); 
    tc.set_value("new_filename", std::string("jkasdfjksjadf"));           
    tc.set_value("temp_dir", std::string("jkasdfjksjadf"));            
    tc.set_value("working_dir", std::string("jkasdfjksjadf"));      
    tc.set_value("config_dir", std::string("jkasdfjksjadf"));        
    
    std::cout<<" SIZEOF : " << sizeof(tc) << std::endl;
    std::string test;
    test += "kksdajfkajsdlfjasjdkjflkasjdkfjkasjdfjaksjdfkjaskdjfsadfs"; 
    std::cout<< " SIZEOF : std::string : " << test.size() << std::endl;


}
*/

TEST(BASE64, ENCODEDECODE) {
    std::string b("JQselvZZ6QBWutuaRg13TQ==");

    for(int i=0; i<100; i++) {
        std::string d;
        attic::crypto::Base64DecodeString(b, d);

        std::string e;
        attic::crypto::Base64EncodeString(d, e);

    }
}

TEST(BASE64, TEST) {
    CryptoPP::AutoSeededRandomPool  g_Rnd;

    for(int i=0; i<1000; i++) {
        byte iv[CryptoPP::AES::BLOCKSIZE]; 
        g_Rnd.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);
        std::string hold;
        hold.append(reinterpret_cast<const char*>(iv), CryptoPP::AES::BLOCKSIZE);

        std::string output;
        CryptoPP::StringSource(hold, 
                               true, 
                               new CryptoPP::Base64Encoder(new CryptoPP::StringSink(output), 
                                                           false)
                               );

        std::string original;
        CryptoPP::StringSource(output,
                               true,
                               new CryptoPP::Base64Decoder(new CryptoPP::StringSink(original))
                               );

        if(original != hold) {
            std::cout<<" FAIL " << std::endl;
        }
    }
}

TEST(STRTOL, TEST) {
    std::string ts("1370368446");

    time_t tb = strtol(ts.c_str(), NULL, 0);  
    std::cout << " (string) " << ts << std::endl;
    std::cout << " (time_t) " << tb << std::endl;

    time_t e = 1370368446;
    ASSERT_EQ(tb, e);
}



int main (int argc, char* argv[]) {
   int status = 0;
    // Init gtestframework
    testing::InitGoogleTest(&argc, argv);
       
    // All tests are setup, run them
    status = RUN_ALL_TESTS();

    return status;
}
