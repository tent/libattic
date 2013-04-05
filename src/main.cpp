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

// Globals
std::string g_Entity;

std::string g_appName;
bool g_bRegApp = false;
TEST(APP_REGISTRATION, STARTAPPINST) {
    if(g_Entity.empty()) return;
    if(!g_bRegApp) return;

    std::cout<<" got here " << std::endl;

    char* buf = new char[g_Entity.size()+1];
    memset(buf, '\0', g_Entity.size()+1);
    memcpy(buf, g_Entity.c_str(), g_Entity.size());

    std::string name("libattic");
    name += " ";
    name += g_appName;

    int status = RegisterAtticApp(g_Entity.c_str(),
                                  name.c_str(), 
                                  "LibAttic Test Suite", 
                                  "www.tent.is", 
                                  "", 
                                  g_Entity.c_str(),
                                  "./config");

    if(status != attic::ret::A_OK) {
        std::cout<<"Startup app instance FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, attic::ret::A_OK);

    /*
    status = RegisterApp(g_Entity.c_str(), "./config");
    if(status != attic::ret::A_OK)
    {
        std::cout<<"register app FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, attic::ret::A_OK);
    */
    
    std::cout<< GetAuthorizationURL() << std::endl;
    
}

std::string g_AuthCode;
bool g_bReqAuthDetails = false;
TEST(APP_REGISTRATION, REQUEST_AUTH_DETAILS)
{
    if(g_Entity.empty()) return;
    if(!g_bReqAuthDetails) return;
    
    ASSERT_EQ(RequestUserAuthorizationDetails(g_Entity.c_str(), g_AuthCode.c_str(), "./config"), attic::ret::A_OK);
}

void TemporaryKeyCb(int a, int b, const char* szC) {
    std::cout<<"TEMPORARY KEY : " << szC << std::endl;

    //int status = EnterPassphrase(szC);

    //std::cout<<"ENTERING PASSHPRASE : " << status << std::endl;
}

void RecoveryKeyCb(int a, int b, const char* szC) {
    std::cout<<"*******************************************"<<std::endl;
    std::cout<<" KEY : " << szC << std::endl;

    //std::cout<<" entering recovery key ... " << std::endl;
    //int status = EnterRecoveryKey(szC);
    //ASSERT_EQ(status, attic::ret::A_OK);
}

bool g_bPassphrase = false;
std::string g_Passphrase;
TEST(PASSPHRASE, REGISTER)
{
    if(g_Entity.empty()) return;
    if(!g_bPassphrase) return;

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();
    
    std::cout<<" LIB INITIALIZED " << std::endl;
    ASSERT_EQ(status, attic::ret::A_OK);
   
    if(status == attic::ret::A_OK) {
        RegisterForTemporaryKeyNotify(&TemporaryKeyCb);
        RegisterForRecoveryKeyNotify(&RecoveryKeyCb);
        status = RegisterPassphrase(g_Passphrase.c_str(), true);
        //ASSERT_EQ(status, attic::ret::A_OK);

        std::cout<< " REGISTER STATUS : " << status << std::endl;
        int count =0;
        for(;;) {
            if(count >=15)
                break;
            sleep(1);
            count++;
        }
    }
    else
        std::cout<<" FAILED TO INIT : " << status << std::endl;

    ShutdownLibAttic(NULL);
}

bool g_bEnterPass = false;

TEST(PASSPHRASE, ENTER)
{
    if(g_Entity.empty()) return;
    if(!g_bEnterPass) return;

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();
    //status = EnterPassphrase("password");
    //status = EnterPassphrase("asdf");
    status = EnterPassphrase(g_Passphrase.c_str());

    std::cout<<" Enter passphrase status : " << status << std::endl;
    ASSERT_EQ(status, attic::ret::A_OK);

   sleep(10);

    ShutdownLibAttic(NULL);
}

bool g_bChangePass = false;
TEST(PASSPHRASE, CHANGE)
{
    if(g_Entity.empty()) return;
    if(!g_bChangePass) return;

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();


    //status = ChangePassphrase("UldG3H35RSMo1jqx", "password");
    std::cout<<" Changing password to \"thispass\""<<std::endl;
    status = ChangePassphrase("password", "thispass");
    std::cout<<" Change passphrase status : " << status << std::endl;
    ASSERT_EQ(status, attic::ret::A_OK);
    status = EnterPassphrase("thispass");
    ASSERT_EQ(status, attic::ret::A_OK);
    std::cout<<" Changing password back " << std::endl;
    status = ChangePassphrase("thispass", "password");
    ASSERT_EQ(status, attic::ret::A_OK);
    status = EnterPassphrase("password");
    ASSERT_EQ(status, attic::ret::A_OK);

   sleep(10);

    ShutdownLibAttic(NULL);
}

bool g_bRecover = false;
std::string g_recoverykey;
TEST(PASSPHRASE, RECOVER)
{
    if(g_Entity.empty()) return;
    if(g_recoverykey.empty()) return;
    if(!g_bRecover) return;

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    RegisterForTemporaryKeyNotify(&TemporaryKeyCb);
    status = EnterRecoveryKey(g_recoverykey.c_str());

    sleep(10);

    ShutdownLibAttic(NULL);
}

bool g_bPush = false;
std::string g_File;

static void UPLOADSPEEDCB(int a, int b, const char* c) {
    if(c) {
        std::cout<<" SPEED : " << c << "b/s" << std::endl;
    }
}

TEST(AFILE, PUSH)
{
    if(g_Entity.empty()) return;
    if(!g_bPush) return;
    
    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    
    std::cout<<" INIT STATUS : " << status << std::endl;

    ASSERT_EQ(status, attic::ret::A_OK);

    status = EnterPassphrase("password");
   // status = EnterPassphrase("asdf");
    ASSERT_EQ(status, attic::ret::A_OK);

    std::cout<<" register for upload speend notify " << std::endl;
    RegisterForUploadSpeedNotify(&UPLOADSPEEDCB);
    

    if(status == 0) {
        std::string path = "./data/" + g_File;
        status = PushFile(path.c_str());
        std::cout<<" uploading file ... " << std::endl;
        //status = PushFile("./data/videos/posa22.mp4");
        //status = PushFile("./data/music/test.mp3");
        //status = PushFile("./data/oglisv.pdf");
        //status = PushFile("./data/test.txt");
        //status = PushFile("./data/cassandra11.pdf");
        //status = PushFile("./data/algorithmsall.pdf");
        //status = PushFile("./data/freenet.pdf", &PUSHCB);
        //status = PushFile("./data/oa.pdf");
        //status = PushFile("./data/docs/oglisv.pdf", &PUSHCB);
        //status = PushFile("./data/cpp/isoiec.pdf", &PUSHCB);


        ASSERT_EQ(status, attic::ret::A_OK);
    }

    sleep(10);

    std::cout<<"shutting down " << std::endl;
    status = ShutdownLibAttic(NULL);
    std::cout<<" shutdown status : " << status << std::endl;
    ASSERT_EQ(status, attic::ret::A_OK);
}

bool g_bPull = false;

static void DOWNLOADSPEEDCB(int a, int b, const char* c) {
    std::cout<<" DOWNLOADSPEEDCB HIT " << std::endl;
    if(c) {
        std::cout<<" SPEED : " << c << "b/s" << std::endl;
    }
}

TEST(AFILE, PULL)
{
    if(g_Entity.empty()) return;
    if(!g_bPull) return;

    std::string temppath;
    std::string temp("./data/temp");
    attic::fs::GetCanonicalPath(temp, temppath);

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    ASSERT_EQ(status, attic::ret::A_OK);

    std::cout<<" DOWNLOAD SPEED NOTIFY ... " << std::endl;
    RegisterForDownloadSpeedNotify(&DOWNLOADSPEEDCB);

    status = EnterPassphrase("password");
    ASSERT_EQ(status, attic::ret::A_OK);

    if(status == attic::ret::A_OK) {
        std::string rel("./data");
        std::string filepath;
        attic::fs::GetCanonicalPath(rel, filepath);
       // filepath += "/oglisv.pdf";
        filepath += "/" + g_File;

        status = PullFile(filepath.c_str());
        ASSERT_EQ(status, attic::ret::A_OK);
    }

       sleep(15);

    ShutdownLibAttic(NULL);

}

bool g_bDelete = false;

TEST(AFILE, DELETE) 
{
    if(g_Entity.empty()) return;
    if(!g_bDelete) return;

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    ASSERT_EQ(status, attic::ret::A_OK);

    status = EnterPassphrase("password");
    ASSERT_EQ(status, attic::ret::A_OK);

    if(status == attic::ret::A_OK) {
        std::string rel("./data");
        std::string filepath;
        attic::fs::GetCanonicalPath(rel, filepath);
        filepath += "/oglisv.pdf";

        status = DeleteFile(filepath.c_str());
        ASSERT_EQ(status, attic::ret::A_OK);
    }

       sleep(10);

    ShutdownLibAttic(NULL);
}

bool g_bManifest = false;
void Filecb(int code, char** pList, int stride, int totes)
{
    std::cout<<" CODE " << code << std::endl;
    std::cout<<" totes : " << totes << std::endl;
    for(int i=0; i<stride; i++)
        std::cout<< pList[i] << std::endl;

    FreeFileList(pList, stride);
}

TEST(MANIFEST, QUERY_ALL_FILES)
{
    if(g_Entity.empty()) return;
    if(!g_bManifest) return;

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    ASSERT_EQ(status, attic::ret::A_OK);
    GetFileList(Filecb);

   sleep(10);

    ShutdownLibAttic(NULL);
}

TEST(INIT, SHUTDOWN)
{

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    ASSERT_EQ(status, attic::ret::A_OK);

    ShutdownLibAttic(NULL);
}

#include "clientutils.h"
bool g_bDiscover = false;
TEST(DISCOVERY, OUTWARD_DISCOVERY)
{
    if(g_Entity.empty()) return;
    if(!g_bDiscover) return;

    std::cout<<" init ? " << std::endl;

    attic::Entity ent;
    attic::client::Discover(g_Entity, NULL, ent);

    /*
    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    std::cout<<" Getting entity api root ... " << std::endl;

    ASSERT_EQ(status, attic::ret::A_OK);
   // std::cout<<" ENTITY API ROOT : " << GetEntityApiRoot(g_Entity.c_str()) << std::endl;

    for(;;)
    {
       sleep(10);
       if(!g_ThreadCount)
           break;
       std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
    }

    ShutdownLibAttic(NULL);
    */

}

bool g_bSync = false;

TEST(TEST, SYNC)
{
    if(g_Entity.empty()) return;
    if(!g_bSync) return;

    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    if(status == attic::ret::A_OK) {

        EnterPassphrase("password");
        std::cout<<"syncing..."<<std::endl;
     //   status = SyncFiles(SYNCCB);
        status = PollFiles();
        std::cout<<"done calling ... " << std::endl;
    }

       sleep(10);

    ShutdownLibAttic(NULL);
}

// Non command driven tests //

TEST(PROCESS, COMPRESS_ENCRYPT_DECRYPT_COMPRESS)
{
    std::string test("This is a test string, of some sort of data, it's pretty great");
    std::cout<<" TESTING COMPRESSION " << std::endl;

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

    ASSERT_EQ(cred.KeyEmpty(), true);
    ASSERT_EQ(cred.IvEmpty(), true);

    cred = attic::crypto::GenerateCredentials();

    ASSERT_EQ(cred.KeyEmpty(), false);
    ASSERT_EQ(cred.IvEmpty(), false);
}

TEST(CRYPTO, SIZES)
{
    std::cout<< " MAXKEYLEN : " << CryptoPP::AES::MAX_KEYLENGTH << std::endl;
    std::cout<< " BLOCKSIZE : " << CryptoPP::AES::BLOCKSIZE << std::endl;
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

TEST(REINTERPREST, CAST)
{
    byte bkey[CryptoPP::AES::MAX_KEYLENGTH];
    byte bkey1[CryptoPP::AES::MAX_KEYLENGTH];
    
    std::string key("whatkjdfjsdkajfsk");

    memcpy( bkey, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            CryptoPP::AES::MAX_KEYLENGTH);

    memcpy( bkey1, 
            reinterpret_cast<const unsigned char*>(key.c_str()), 
            CryptoPP::AES::MAX_KEYLENGTH);

    int res =  strcmp( reinterpret_cast<const char*>(bkey), 
                       reinterpret_cast<const char*>(bkey1));
    ASSERT_EQ(res, 0);
}

TEST(FILESYSTEM, RELATIVETO)
{
    std::string relative;
    attic::fs::MakePathRelative("foo/bar", "this/foo/test/something/what.txt", relative);
    ASSERT_EQ(relative, std::string("../../this/foo/test/something/what.txt"));
}

TEST(FILESYSTEM, GETCANONICALPATH)
{
    std::string path("./data/oglisv.pdf");
    std::string absolute;
    attic::fs::GetCanonicalPath(path, absolute);
}

TEST(AUTHCODE, GENERATE)
{
    std::string authcode;
    attic::utils::GenerateRandomString(authcode, 32);
    std::cout<<" AUTH CODE : " << authcode << std::endl;
    std::string encoded;
    attic::crypto::Base32EncodeString(authcode, encoded);
    std::cout<<" ENCODED : " << encoded << std::endl;
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

    std::cout<<" SERIALIZED : " << output << std::endl;

    attic::ChunkInfo ci2;
    attic::jsn::DeserializeObject(&ci2, output);
    

    std::string output2;
    attic::jsn::SerializeObject(&ci2, output2);

    ASSERT_EQ(output, output2);
}

TEST(FOLDER, SERIALIZATION)
{
    attic::Folder folder;
    attic::FolderEntry one("dasf", "file", "adsfae");
    attic::FolderEntry two("einen", "file", "914891284192jfkjadkfe");
    folder.PushBackEntry(one);
    folder.PushBackEntry(two);

    std::string output;
    attic::jsn::SerializeObject(&folder, output);

    attic::Folder other;
    attic::jsn::DeserializeObject(&other, output);

    std::string output2;
    attic::jsn::SerializeObject(&other, output2);

    ASSERT_EQ(output, output2);

    attic::FolderEntry three("fdasfsad", "1111", "2222222");
    folder.PushBackEntry(three);
    std::string tt; 
    attic::jsn::SerializeObject(&folder, tt);
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


bool g_bRollsum = false;
std::string g_filepath;
TEST(ROLLSUM, FILETEST)
{
    if(g_filepath.empty()) return;
    if(!g_bRollsum) return;

    std::cout<<" opening file : " << g_filepath << std::endl;
    std::ifstream ifs;
    ifs.open(g_filepath.c_str(), std::ios::in  | std::ios::binary);

    int totalsize = 0;

    int splitmin = 2000000;
    int splitmax = 8000000;

    int buffersize = 10000000;

    const unsigned int filesize = attic::utils::CheckFilesize(g_filepath);

    std::ofstream ofs;
    ofs.open("out.mp4", std::ios::out | std::ios::binary);

    if(ifs.is_open()) {
        std::cout<<" begining read " << std::endl;
        // Setup window buffer
        std::string window, remainder;
        unsigned int readcount = 0;
        unsigned int chunkcount = 0;
        attic::RollSum rs;
        while(!ifs.eof()) {
            char* pData = NULL;
            unsigned int datasize = 0;
            /*
            if(filesize >= attic::cnst::g_unMaxBuffer) {
                pData = new char[filesize];
                datasize = filesize;
            }
            else {
            */
                if((filesize - readcount) <= attic::cnst::g_unMaxBuffer) {
                    pData = new char[(filesize - readcount)];
                    datasize = (filesize - readcount);
                }
                else {
                    pData = new char[attic::cnst::g_unMaxBuffer];
                    datasize = attic::cnst::g_unMaxBuffer;
                }
            //}
            std::cout<<" datasize : " << datasize << std::endl;
            
            ifs.read(pData, datasize);
            readcount += datasize;

            window.clear();
            window += remainder;
            remainder.clear();

            if(datasize)
                window.append(pData, datasize);

            if(pData) {
                delete[] pData;
                pData = NULL;
            }

            // find splits
            int totalread = 0, count = 0, lastsplit = 0;
            for(unsigned int i=0; i<window.size(); i++) {
                char c = window[i];
                count++;
                rs.Roll(c);
                if(rs.OnSplit()) {
                    if(count >= attic::cnst::g_unSplitMin) {
                        std::string chunk;
                        int diff = i - lastsplit;
                        chunk = window.substr(lastsplit, diff);
                        // Transform
                     //   SendChunk( chunk, fileKey, boundary, ssl_sock, count, false, pFi);
                        lastsplit = i;
                        count = 0;
                        chunkcount++;
                        totalread += chunk.size();

                        std::string plaintextHash;
                        attic::crypto::GenerateHash(chunk, plaintextHash);
                        std::cout<<" hash : " << plaintextHash << std::endl;

                        //std::cout<<" split found at : " << totalread << std::endl;
                        ofs.write(chunk.c_str(), chunk.size());
                    }
                }

                if(count >= attic::cnst::g_unSplitMax) {
                    std::string chunk;
                    int diff = i - lastsplit;
                    chunk = window.substr(lastsplit, diff);
                    // Transform
                    //SendChunk( chunk, fileKey, boundary, ssl_sock, count, false, pFi);
                    lastsplit = i;
                    count = 0;
                    chunkcount++;
                    totalread += chunk.size();

                    std::string plaintextHash;
                    attic::crypto::GenerateHash(chunk, plaintextHash);
                    std::cout<<" hash : " << plaintextHash << std::endl;
                    //std::cout<<" split found at : " << totalread << std::endl;
                    //
                    ofs.write(chunk.c_str(), chunk.size());
                }
            }

            // Set remainder
            int wdiff = window.size() - totalread;
            if(wdiff)
                remainder = window.substr(lastsplit, wdiff);

            if((readcount + remainder.size()) >= filesize) {

                std::string plaintextHash;
                attic::crypto::GenerateHash(remainder, plaintextHash);
                std::cout<<" hash : " << plaintextHash << std::endl;
                ofs.write(remainder.c_str(), remainder.size());

                std::cout<<" FILE SIZE : " << filesize << std::endl;
                std::cout<<" READCOUNT : " << readcount << std::endl;
                std::cout<<" REMAINDER : " << remainder.size() << std::endl;
                
                //std::cout<<" split found at : " << readcount +  << std::endl;
                //SendChunk( remainder, fileKey, boundary, ssl_sock, chunkcount, true, pFi);
                chunkcount++;
                break;
            }

        }
        std::cout<<" rolling file done " << std::endl;
    }

    
}

bool g_bDaemon = false;
TEST(ATTIC, DAEMON)
{
    if(g_Entity.empty()) return;
    if(!g_bDaemon) return;

    std::cout<<" Initializing daemon ... " << std::endl;
    
    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("entity_url", g_Entity.c_str());
    int status = InitLibAttic();

    std::cout<<" status : " << status << std::endl;
    ASSERT_EQ(status, attic::ret::A_OK);

    std::cout<<" entering passphrase ... " << std::endl;
    //status = EnterPassphrase("asdf");

    status = EnterPassphrase("password");
    std::cout<<" status : " << status << std::endl;
    ASSERT_EQ(status, attic::ret::A_OK);

    //std::cout<<" starting to poll ... " << std::endl;
    status = PollFiles();
    //std::cout<<" running til interrupt ... " << std::endl;
    while(true)
        sleep(10);

    ShutdownLibAttic(NULL);
}

TEST(FILEINFO, POSTVERSION)
{
    attic::FileInfo fi;
    std::cout<<" POST VERSION : " << fi.post_version() << std::endl;

    fi.set_post_version("12");
    std::cout<<" POST VERSION : " << fi.post_version() << std::endl;
}

TEST(FILESYSTEM, SCAN)
{
    std::vector<std::string> paths;
    attic::fs::ScanDirectory("./data", paths);
    for(unsigned int i=0; i<paths.size(); i++) {
        std::cout<<paths[i]<<std::endl;
    }
}

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


int main (int argc, char* argv[]) {
   int status = 0;

    if(argc > 1) {
        int optcount = 14;
        char* options[] = {
            "REGISTERAPP",
            "REQUESTAUTHCODE",
            "REGISTERPASS",
            "ENTERPASS",
            "CHANGEPASS",
            "PULL",
            "PUSH",
            "DELETE",
            "SYNC",
            "QUERYMANIFEST",
            "DISCOVER",
            "SPLITTEST",
            "RECOVER",
            "DAEMON",
            };

        enum ecmd
        {
            REGISTERAPP=0,
            REQUESTAUTHCODE,
            REGISTERPASS,
            ENTERPASS,
            CHANGEPASS,
            PULL,
            PUSH,
            DELETE,
            SYNC,
            QUERYMANIFEST,
            DISCOVER,
            SPLITTEST,
            RECOVER,
            DAEMON
        };

        if(!strcmp(argv[1], "--help")) {
            std::cout<<" Usage : ./attic <OPTION> {<OPTION ARG>} <ENTITY>" << std::endl;
            std::cout<<" Options : "<< std::endl;
            for(int i=0; i<optcount; i++) {
                std::cout << "\t" << options[i] << std::endl;
            }
            return 0;
        }

        // Init gtestframework
        testing::InitGoogleTest(&argc, argv);
        if(argc > 2) {
            // Extract entity
            g_Entity = argv[argc-1];

            int opt=-1;
            // Extract command
            for(int i=0; i < optcount; i++) {
                if(!strcmp(options[i], argv[1])) {
                    opt = i;
                    break;
                }
            }

            std::cout<<" OPT : " << opt << std::endl;

            if(opt!=-1) {
                switch(opt) {
                    case REGISTERAPP:
                    {
                        if(argc > 3) {
                            g_appName = argv[2];
                            g_bRegApp = true;
                        }
                        else {
                            std::cout<<" Invalid params, ./attic REGISTERAPP <app name> <entity>" << std::endl;
                        }
                        break;
                    }
                    case REQUESTAUTHCODE:
                    {
                        if(argc > 3) {
                            g_AuthCode = argv[2];
                            g_bReqAuthDetails = true;
                        }
                        else
                            std::cout<<" Invalid params, ./attic REQUESTAUTHCODE <code> <entity> " << std::endl;
                        break;
                    }
                    case REGISTERPASS:
                    {
                        if(argc > 3) {
                            g_Passphrase = argv[2];
                            g_bPassphrase = true;
                        }
                        else
                            std::cout<<" Invalid params, ./attic REGISTERPASS <password> <entity> " << std::endl;

                        break;
                    }
                    case ENTERPASS:
                    {
                        if(argc > 3) {
                            g_Passphrase = argv[2];
                            g_bEnterPass = true;
                        }
                        else
                            std::cout<<" Invalid params, ./attic ENTERPASS <password> <entity> " << std::endl;

                        break;
                    }
                    case CHANGEPASS:
                    {
                        g_bChangePass = true;
                        break;
                    }
                    case PULL:
                    {
                        if(argc > 3) {
                            g_bPull = true;
                            g_File = argv[2];
                        }
                        else {
                            std::cout<<" Invalid params, ./attic PUSH <filename> (file is relative to data dir) " << std::endl;
                        }
                        break;
                    }
                    case PUSH:
                    {
                        if(argc > 3) {
                            g_bPush = true;
                            g_File = argv[2];
                        }
                        else {
                            std::cout<<" Invalid params, ./attic PUSH <filename> (file is relative to data dir) " << std::endl;
                        }
                        break;
                    }
                    case DELETE:
                    {
                        g_bDelete = true;
                        break;
                    }
                    case SYNC:
                    {
                        g_bSync = true;
                        break;
                    }
                    case QUERYMANIFEST:
                    {
                        g_bManifest = true;
                        break;
                    }
                    case DISCOVER:
                    {
                        g_bDiscover = true;
                        break;
                    }
                    case SPLITTEST:
                    {

                        if(argc > 2) {
                            g_filepath = argv[2];
                            g_bRollsum = true;
                        }
                        else
                            std::cout<<" Invalid params ";
                        break;
                    }
                    case RECOVER:
                    {
                        if(argc >2 ) {
                            g_recoverykey = argv[2];
                            g_bRecover = true;
                        }
                        else
                            std::cout<<" Invalid params ";
                        break;
                    }
                    case DAEMON:
                    {
                        g_bDaemon = true;
                        break;
                    }
                    default:
                        break;
                }

            }
        }
        
        // All tests are setup, run them
        status = RUN_ALL_TESTS();
    }


    return status;
}
