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
#include "log.h"

#include "netlib.h"
#include "compression.h"

#include "filesystem.h"
#include "folder.h"
#include "folderpost.h"

#include "rollsum.h"
#include "atticpost.h"

// Globals
std::string g_Entity;

bool g_bRegApp = false;
TEST(APP_REGISTRATION, STARTAPPINST)
{
    if(g_Entity.empty()) return;
    if(!g_bRegApp) return;

    std::cout<<" got here " << std::endl;

    char* buf = new char[g_Entity.size()+1];
    memset(buf, '\0', g_Entity.size()+1);
    memcpy(buf, g_Entity.c_str(), g_Entity.size());

    char* p[] = { buf };
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


    int status = StartupAppInstance( "libattic", 
                                     "LibAttic Test Suite", 
                                     "www.tent.is", 
                                     "", 
                                     p,
                                     1, 
                                     s, 
                                     18);
    if(status != ret::A_OK)
    {
        std::cout<<"Startup app instance FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    status = RegisterApp(g_Entity.c_str(), "./config");
    if(status != ret::A_OK)
    {
        std::cout<<"register app FAILED : " << status << std::endl;
    }
    ASSERT_EQ(status, ret::A_OK);

    status = RequestAppAuthorizationURL(g_Entity.c_str());
    if(status != ret::A_OK)
    {
        std::cout<<"Request app authorization URL FAILED : " << status << std::endl;
    }
    std::cout<< GetAuthorizationURL() << std::endl;
    ASSERT_EQ(status, ret::A_OK);
    
    if(buf)
    {
        delete buf;
        buf =NULL;
    }
}

std::string g_AuthCode;
bool g_bReqAuthDetails = false;
TEST(APP_REGISTRATION, REQUEST_AUTH_DETAILS)
{
    if(g_Entity.empty()) return;
    if(!g_bReqAuthDetails) return;
    
    ASSERT_EQ(RequestUserAuthorizationDetails(g_Entity.c_str(), g_AuthCode.c_str(), "./config"), ret::A_OK);
}

bool g_bPassphrase = false;
TEST(PASSPHRASE, REGISTER)
{
    if(g_Entity.empty()) return;
    if(!g_bPassphrase) return;

    int status = InitLibAttic( 
                  "./data",
                  "./config",
                  "./data/temp",
                  "./config/logs",
                  g_Entity.c_str());
   
    if(status == ret::A_OK)
    {
        status = RegisterPassphrase("password", true);

        ASSERT_EQ(status, ret::A_OK);
   
        std::cout<< " REGISTER STATUS : " << status << std::endl;
        for(;;)
        {
            sleep(10);
            if(!g_ThreadCount)
                break;
            std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
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

    int status = InitLibAttic( 
                  "./data",
                  "./config",
                  "./data/temp",
                  "./config/logs",
                  g_Entity.c_str());


    status = EnterPassphrase("password");
    std::cout<<" Enter passphrase status : " << status << std::endl;
    ASSERT_EQ(status, ret::A_OK);

    for(;;)
    {
       sleep(10);
       if(!g_ThreadCount)
           break;
       std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
    }

    ShutdownLibAttic(NULL);
}

bool g_bPush = false;

static void PUSHCB(int a, void* b)
{
    std::cout<<" PUSH CALLBACK : " << a << std::endl;
}

TEST(PUSH, AFILE)
{
    if(g_Entity.empty()) return;
    if(!g_bPush) return;
    
    int status = InitLibAttic( 
                  "./data",
                  "./config",
                  "./data/temp",
                  "./config/logs",
                  g_Entity.c_str());
    
    std::cout<<" INIT STATUS : " << status << std::endl;

    ASSERT_EQ(status, ret::A_OK);

    status = EnterPassphrase("password");
    ASSERT_EQ(status, ret::A_OK);

    if(status == 0)
    {
   //     status = PushFile("./data/oglisv.pdf", &PUSHCB);
        status = PushFile("./data/ccf.pdf", &PUSHCB);

        ASSERT_EQ(status, ret::A_OK);
    }

    for(;;)
    {
       //std::cout<<" UPLOAD SPEED : " << GetActiveUploadSpeed() << std::endl;
       //std::cout<< "PUSH TASK COUNT : " << GetActivePushTaskCount() << std::endl;
       //std::cout<<" UPLOAD SPEED : " << GetActiveUploadSpeed() << std::endl;

       sleep(10);
       if(!g_ThreadCount)
           break;
       std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
    }

    ShutdownLibAttic(NULL);
}

bool g_bPull = false;
void PULLCB(int a, void* b)
{
    std::cout<<" CALLBACK HIT BRAH : " << a << std::endl;

}
TEST(PULL, AFILE)
{
    if(g_Entity.empty()) return;
    if(!g_bPull) return;

    int status = InitLibAttic( 
                  "./data",
                  "./config",
                  "./data/temp",
                  "./config/logs",
                  g_Entity.c_str());

    ASSERT_EQ(status, ret::A_OK);

    status = EnterPassphrase("password");
    ASSERT_EQ(status, ret::A_OK);

    if(status == ret::A_OK) {
        status = PullFile("oglisv.pdf", &PULLCB);
        //status = PullFile("./data/oa.pdf", &PUSHCB);
        ASSERT_EQ(status, ret::A_OK);
    }

    for(;;)
    {
       sleep(10);
       if(!g_ThreadCount)
           break;
       std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
    }

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

    int status = InitLibAttic( 
                  "./data",
                  "./config",
                  "./data/temp",
                  "./config/logs",
                  g_Entity.c_str());

    ASSERT_EQ(status, ret::A_OK);
    GetFileList(Filecb);

    for(;;)
    {
       sleep(10);
       if(!g_ThreadCount)
           break;
       std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
    }

    ShutdownLibAttic(NULL);
}


bool g_bDiscover = false;
TEST(DISCOVERY, OUTWARD_DISCOVERY)
{
    if(g_Entity.empty()) return;
    if(!g_bDiscover) return;

    std::cout<<" init ? " << std::endl;

    int status = InitLibAttic( 
                  "./data",
                  "./config",
                  "./data/temp",
                  "./config/logs",
                  g_Entity.c_str());

    std::cout<<" Getting entity api root ... " << std::endl;

    ASSERT_EQ(status, ret::A_OK);
    std::cout<<" ENTITY API ROOT : " << GetEntityApiRoot(g_Entity.c_str()) << std::endl;

    for(;;)
    {
       sleep(10);
       if(!g_ThreadCount)
           break;
       std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
    }

    ShutdownLibAttic(NULL);

}

bool g_bSync = false;
void SYNCCB(int a, void* b)
{
    std::cout<<" SYNC CALLBACK HIT : " << a << std::endl;

}
TEST(TEST, SYNC)
{
    if(g_Entity.empty()) return;
    if(!g_bSync) return;

    int status = InitLibAttic( 
                  "./data",
                  "./config",
                  "./data/temp",
                  "./config/logs",
                  g_Entity.c_str());

    if(status == ret::A_OK) {

        EnterPassphrase("password");
        std::cout<<"syncing..."<<std::endl;
        status = SyncFiles(SYNCCB);
        std::cout<<"done calling ... " << std::endl;
    }

    for(;;) {
       sleep(10);
       if(!g_ThreadCount)
           break;
       std::cout<<"MAIN Thread count : " << g_ThreadCount << std::endl;
    }

    ShutdownLibAttic(NULL);
}

// Non command driven tests //

TEST(PROCESS, COMPRESS_ENCRYPT_DECRYPT_COMPRESS)
{
    std::string test("This is a test string, of some sort of data, it's pretty great");

    // Compress
    std::string compressed;
    compress::CompressString(test, compressed);
    // Encrypt
    Credentials cred = crypto::GenerateCredentials();
    std::string encrypted;
    crypto::EncryptStringCFB(compressed, cred, encrypted);
    // Decrypt
    std::string decrypted;
    crypto::DecryptStringCFB(encrypted, cred, decrypted);
    // Decompress
    std::string decompressed;
    compress::DecompressString(decrypted, decompressed);

    ASSERT_EQ(test, decompressed);

}

TEST(COMPRESS, COMPRESSSTRING)
{
    std::string in("this is my test string it is a pretty decent test string");
    std::string out;

    compress::CompressString(in, out);

    std::string decomp;
    compress::DecompressString(out, decomp);

    ASSERT_EQ(in, decomp);
}

TEST(NETLIB, EXTRACTHOSTANDPATH)
{
    std::string url = "https://manuel.tent.is/tent/posts";
    std::string protocol, host, path;

    netlib::ExtractHostAndPath(url, protocol, host, path);
    ASSERT_EQ(host, std::string("manuel.tent.is"));
    ASSERT_EQ(path, std::string("/tent/posts"));
}

TEST(CREDENTIALS, ISEMPTY)
{
    Credentials cred;

    ASSERT_EQ(cred.KeyEmpty(), true);
    ASSERT_EQ(cred.IvEmpty(), true);

    cred = crypto::GenerateCredentials();

    ASSERT_EQ(cred.KeyEmpty(), false);
    ASSERT_EQ(cred.IvEmpty(), false);
}

TEST(CRYPTO, BASE64)
{
    std::string teststring("this is my test string, that I'm going to base64 encode");
    std::string encoded;
    crypto::Base64EncodeString(teststring, encoded);
    std::string decoded;
    crypto::Base64DecodeString(encoded, decoded);
    ASSERT_EQ(teststring, decoded);
}

TEST(CRYPTO, HMAC)
{
    int status = ret::A_OK;
    Credentials cred = crypto::GenerateCredentials();

    std::string plaintext("this is my plain text");

    std::string macout;
    status = crypto::GenerateHMACForString( plaintext, cred, macout);
    ASSERT_EQ(status, ret::A_OK);

    status = crypto::VerifyHMACForString( plaintext, cred, macout);
    ASSERT_EQ(status, ret::A_OK);
}

TEST(CRYPTO, ENCRYPTIONCFB)
{
    Credentials cred = crypto::GenerateCredentials();

    std::string plaintext("this is my plain text");

    std::string cyphertext;
    crypto::EncryptStringCFB(plaintext, cred, cyphertext);

    std::string decryptedtext;
    crypto::DecryptStringCFB(cyphertext, cred, decryptedtext);

    ASSERT_EQ(plaintext, decryptedtext);
}

TEST(CRYPTO, CREDENCRYPTIONGCM)
{
    Credentials masterkey;

    std::string phrase("this is a test");
    std::string iv;
    crypto::GenerateSalt(iv);

    // Genterate key from passphrase
    int status = crypto::GenerateKeyFromPassphrase( phrase,
                                               iv,
                                               masterkey);
    ASSERT_EQ(status, 0);

    Credentials cred; // Credentials to encrypt
    cred = crypto::GenerateCredentials();

    std::string key;
    cred.GetKey(key);

    Credentials intercred; // credentials used to encrypt file key
                           // master key
                           // file specific iv

    std::string mk, fileiv;
    masterkey.GetKey(mk);
    cred.GetIv(fileiv);

    intercred.SetKey(mk);
    intercred.SetIv(fileiv);

    std::string enckey;
    status = crypto::EncryptStringGCM(key, intercred, enckey);
    ASSERT_EQ(status, 0);

    // Generate key again for good measure
    Credentials mkcopy;
    status = crypto::GenerateKeyFromPassphrase( phrase,
                                           iv,
                                           mkcopy);
    ASSERT_EQ(status, 0);

    Credentials intercred1;
    std::string mk1;
    mkcopy.GetKey(mk1);

    intercred1.SetKey(mk1);
    intercred1.SetIv(fileiv);

    std::string deckey;
    status = crypto::DecryptStringGCM(enckey, intercred1, deckey);
    ASSERT_EQ(status, 0);

    ASSERT_EQ(key, deckey);
}

TEST(SCRYPT, ENTER_PASSPHRASE)
{
    Credentials cred, cred1;

    std::string pw("password");
    std::string iv;
    crypto::GenerateSalt(iv); 

    int status = crypto::GenerateKeyFromPassphrase( pw,
                                               iv,
                                               cred);
    
    ASSERT_EQ(status, 0);
    status = crypto::GenerateKeyFromPassphrase( pw ,
                                  iv,
                                  cred1);

    ASSERT_EQ(status, 0);
    ASSERT_EQ(cred.GetKey(), cred1.GetKey());
    ASSERT_EQ(cred.GetIv(), cred1.GetIv());
}

TEST(SCRYPT, ENCODE)
{
    std::string input("thisistestinput");
    std::string iv;

    crypto::GenerateSalt(iv); 

    std::string out, out1;
    crypto::ScryptEncode(input, iv, out, CryptoPP::AES::MAX_KEYLENGTH);
    crypto::ScryptEncode(input, iv, out1, CryptoPP::AES::MAX_KEYLENGTH);

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
    fs::MakePathRelative("foo/bar", "this/foo/test/something/what.txt", relative);
    ASSERT_EQ(relative, std::string("../../this/foo/test/something/what.txt"));
}

TEST(FILESYSTEM, GETCANONICALPATH)
{
    std::string path("./data/oglisv.pdf");
    std::string absolute;
    fs::GetCanonicalPath(path, absolute);
}

/*
TEST(MANIFEST, INSERTFILEINFO)
{
    FileInfo fi( "somefile.pdf",
                 "foo/bar/somefile.pdf",
                 "kjsadkfj-3412",
                 "ASDFf=342jf#-_");

    Manifest mf;
    std::string dir("");
    mf.SetDirectory(dir);
    int status = mf.Initialize();
    ASSERT_EQ(status, ret::A_OK);

    ASSERT_EQ(mf.InsertFileInfo(fi), true);

    mf.Shutdown();
}

TEST(MANIFEST, INSERTFILEPOSTID)
{
    Manifest mf;
    std::string dir("");
    mf.SetDirectory(dir);
    int status = mf.Initialize();
    ASSERT_EQ(status, ret::A_OK);

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
    ASSERT_EQ(status, ret::A_OK);

    std::string filepath("foo/bar/somefile.pdf");
    ASSERT_EQ(mf.RemoveFileInfo(filepath), true);

    mf.Shutdown();
}
*/

TEST(FOLDER, SERIALIZATION)
{
    Folder folder;
    FolderEntry one("dasf", "file", "adsfae");
    FolderEntry two("einen", "file", "914891284192jfkjadkfe");
    folder.PushBackEntry(one);
    folder.PushBackEntry(two);

    std::string output;
    jsn::SerializeObject(&folder, output);

    Folder other;
    jsn::DeserializeObject(&other, output);

    std::string output2;
    jsn::SerializeObject(&other, output2);

    std::cout<<" OUTPUT : " << output << std::endl;

    ASSERT_EQ(output, output2);

    FolderPost p(folder);
    std::string fp;
    jsn::SerializeObject(&p, fp);

    std::cout<<" FOLDER POST : " << fp << std::endl;

    FolderPost pp;
    jsn::DeserializeObject(&pp, fp);

    Permissions perm;
    std::string permout;
    jsn::SerializeObject(&perm, permout);

    std::cout<<" permissions out : " << permout << std::endl;

    Post post;
    std::string postout;
    jsn::SerializeObject(&post, postout);

    std::cout<<" post out : " << postout << std::endl;


    AtticPost ap;
    std::string atticout;
    jsn::SerializeObject(&ap, atticout);
    std::cout<<" attic out : " << atticout << std::endl;



   
}

bool g_bRollsum = false;
std::string g_filepath;
TEST(ROLLSUM, FILETEST)
{
    if(g_filepath.empty()) return;
    if(!g_bRollsum) return;

    std::ifstream ifs;
    ifs.open(g_filepath.c_str(), std::ios::in  | std::ios::binary);

    int totalsize = 0;

    int splitmin = 2000000;
    int splitmax = 8000000;

    int buffersize = 10000000;

    std::string data, remainder;
    if(ifs.is_open()) {
        ifs.seekg(0, std::ifstream::end);
        totalsize = ifs.tellg();
        ifs.seekg(0, std::ifstream::beg);

        int totalreadcount = 0;
        while(!ifs.eof()) { 
            int buffsize = 3000000;
            char* pData = NULL;
            int readcount = 0;
            if(totalsize > buffsize) {
                pData = new char[buffsize];
                ifs.read(pData, buffsize);
                readcount = buffsize;
            }
            else { 
                pData = new char[totalsize];
                ifs.read(pData, totalsize);
                data.append(pData, totalsize);
                readcount = totalsize;
            }
            data.clear();
            data += remainder;
            remainder.clear();
            data.append(pData, readcount);
            totalreadcount += readcount;

            if(pData) {
                delete pData;
                pData = NULL;
            }
            // Go through data here with the rollsum
            RollSum s;

            int lastsplit = 0;
            int passing = 0;
            for(unsigned int i=0; i < data.size(); i++) {
                char c = data[i];
                s.Roll(c);
                if(s.OnSplit()) { 
                    // check min
                    if(passing > splitmin) { 
                        std::cout<<" found split : " << i << std::endl;

                        passing = 0;
                        lastsplit = i;
                    }
                }

                if(passing >= splitmax) {
                    std::cout<< "split max : " << i << std::endl;

                    passing = 0;
                    lastsplit = i;
                }


                passing++;
            }
            // Get the remainder and append it to remainder
            int df = data.size() - (lastsplit+1);
            std::cout<<" dif : " << df << std::endl;

            if(totalreadcount >= totalsize) break;
        }
             

        ifs.close();
    }

    return;

    std::deque<unsigned int> splits;
    std::cout<<" starting size : " << splits.size() << std::endl;
    std::cout<<" data size : " << data.size() << std::endl;
    std::cout<<" total size : " << totalsize << std::endl;
    if(data.size()) {
        RollSum s;

        int passing = 0;
        for(unsigned int i=0; i < data.size(); i++) {
            char c = data[i];
            s.Roll(c);
            if(s.OnSplit()) { 
                // check min
                if(passing > splitmin) { 
                    std::cout<<" found split : " << i << std::endl;
                    splits.push_back(i);
                    passing = 0;
                }
            }

            if(passing >= splitmax) {
                std::cout<< "split max : " << i << std::endl;
                passing = 0;
                splits.push_back(i);
                passing = 0;
            }

            passing++;
        }

        std::cout<<std::endl;

        ifs.open(g_filepath.c_str(), std::ios::in  | std::ios::binary);

        int runningtotal = 0;
        int last = 0;
        int len = splits.size();
        long long size = 0;
        if(len) {
            for(int j=0; j<len+1; j++) {
                std::cout<<"split : " << splits[j] << std::endl;
                size = 0;
                if(j < len)
                    size = splits[j] - last;
                else {
                    size = totalsize - splits[j-1];
                    std::cout<<" end split : " << splits[j-1] << std::endl;
                }

                runningtotal += size;

                std::cout << " size : " << size << std::endl;

                if(size < 0) { 
                    std::cout << "at split : " << splits[j] << std::endl;
                    std::cout<<" end split : " << splits[len-1] << std::endl;
                }

                char* pData = new char[size];
                ifs.read(pData, size);
                std::string tdata;
                tdata.append(pData, size);

                delete pData;
                pData = NULL;

                std::string hash;
                crypto::GenerateHash(tdata, hash);

                std::cout<<" hash : " << hash << std::endl;

                ifs.seekg(splits[j]);
                last = splits[j];

            }
            ifs.close();

            ASSERT_EQ(runningtotal, totalsize);
        }
    }
}

int main (int argc, char* argv[])
{
    int status = 0;

    if(argc > 1) {
        int optcount = 10;
        char* options[] = {
            "REGISTERAPP",
            "REQUESTAUTHCODE",
            "REGISTERPASS",
            "ENTERPASS",
            "PULL",
            "PUSH",
            "SYNC",
            "QUERYMANIFEST",
            "DISCOVER",
            "SPLITTEST",
            };

        enum ecmd
        {
            REGISTERAPP=0,
            REQUESTAUTHCODE,
            REGISTERPASS,
            ENTERPASS,
            PULL,
            PUSH,
            SYNC,
            QUERYMANIFEST,
            DISCOVER,
            SPLITTEST
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
                        std::cout<<" SET TRUE  " << std::endl;
                        g_bRegApp = true;
                        break;
                    }
                    case REQUESTAUTHCODE:
                    {
                        if(argc > 3)
                        {
                            g_AuthCode = argv[2];
                            g_bReqAuthDetails = true;
                        }
                        else
                            std::cout<<" Invalid params, ./attic REQUESTAUTHCODE <code> <entity> " << std::endl;
                        break;
                    }
                    case REGISTERPASS:
                    {
                        g_bPassphrase = true;
                        break;
                    }
                    case ENTERPASS:
                    {
                        g_bEnterPass = true;
                        break;
                    }
                    case PULL:
                    {
                        g_bPull = true;
                        break;
                    }
                    case PUSH:
                    {
                        g_bPush = true;
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
