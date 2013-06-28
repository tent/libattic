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
#include <ctype.h>

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
#include "fileroller.h"


/*
#define BUFFERSIZE 1000
#include <b64/encode.h>
*/

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


TEST(COMPRESS, MINIZ_COMPRESS_STRING) {
    std::string test_string = "miniz.c employs several proven techniques and approaches used in my much more powerful (but slower and more complex) lossless lzham codec, and in my jpeg-compressor project. Specifically, miniz.c's linear-time Huffman codelength generator, the single switch-statement approach used to implement the decompressor as a single function coroutine, and its very fast Huffman code symbol unpacker are all loosely based off the implementations I wrote for these earlier projects. ";

    std::string out;
    attic::Compress cmp;
    cmp.CompressString(test_string, out);

    std::cout<<" ORIGINAL : " << test_string << std::endl;
    std::cout<<" COMPRESSED : " << out << std::endl;

    std::string dcmp;
    cmp.DecompressString(out, test_string.size(), dcmp);

    std::cout<<" DECOMPRESSED : " << dcmp << std::endl;



}

TEST(COMPRESS, COMPRESSSTRING) {
    std::string in("this is my test string it is a pretty decent test string");
    std::string out;

    attic::compress::CompressString(in, out);

    std::string decomp;
    attic::compress::DecompressString(out, decomp);

    ASSERT_EQ(in, decomp);
}

TEST(NETLIB, EXTRACTHOSTANDPATH) {
    std::string url = "https://manuel.tent.is/tent/posts";
    std::string protocol, host, path;

    attic::netlib::ExtractHostAndPath(url, protocol, host, path);
    ASSERT_EQ(host, std::string("manuel.tent.is"));
    ASSERT_EQ(path, std::string("/tent/posts"));
}

TEST(CREDENTIALS, ISEMPTY) {
    attic::Credentials cred;

    ASSERT_EQ(cred.key_empty(), true);
    ASSERT_EQ(cred.iv_empty(), true);

    cred = attic::crypto::GenerateCredentials();

    ASSERT_EQ(cred.key_empty(), false);
    ASSERT_EQ(cred.iv_empty(), false);
}

TEST(CRYPTO, BASE32) {
    std::string teststring("this is my test string, that I'm going to base32 encode");
    std::string encoded;
    attic::crypto::Base32EncodeString(teststring, encoded);

    std::string decoded;
    attic::crypto::Base32DecodeString(encoded, decoded);
    ASSERT_EQ(teststring, decoded);
}

/*
TEST(CRYPTO, BASE64) {
    std::string teststring("this is my test string, that I'm going to base64 encode");
    std::string payload;
    std::ifstream ifs;
    ifs.open("aho.pdf", std::ios::in | std::ios::binary);
    if(ifs.is_open()) {
        ifs.seekg (0, std::ios::end);
        unsigned int size = ifs.tellg();
        ifs.seekg (0, std::ios::beg);
        std::cout<<" file size : " << size << std::endl;

        char* data = new char[size];
        ifs.read(data, size);
        payload.append(data, size);
        delete data;
        data = NULL;
    }
    std::string encoded;
    attic::crypto::Base64EncodeString(payload, encoded);
    //std::string decoded;
    //attic::crypto::Base64DecodeString(encoded, decoded);
    //ASSERT_EQ(teststring, decoded);
}
*/

/*
TEST(LIBB64, ENCODE) { 
    std::string teststring("this is my test string, that I'm going to base64 encode");

    std::string payload = teststring;
    std::string payload;
    std::ifstream ifs;
    ifs.open("aho.pdf", std::ios::in | std::ios::binary);
    if(ifs.is_open()) {
        ifs.seekg (0, std::ios::end);
        unsigned int size = ifs.tellg();
        ifs.seekg (0, std::ios::beg);
        std::cout<<" file size : " << size << std::endl;

        char* data = new char[size];
        ifs.read(data, size);
        payload.append(data, size);
        delete data;
        data = NULL;
    }

    base64::encoder e;
    std::istringstream str(payload);
    std::ostringstream ostr;
    e.encode(str, ostr);
    //std::cout<<" ENCODED : " << ostr.str() << std::endl;
    //
    std::string encoded;
    attic::crypto::Base64EncodeString(payload, encoded);
    

    ASSERT_EQ(encoded, ostr.str());

}
*/

TEST(CRYPTO, KEY_ENCRYPTION) {

    attic::Credentials cred, master_cred;
    attic::crypto::GenerateCredentials(cred);
    attic::crypto::GenerateCredentials(master_cred);
    // Encrypt
    std::string master_key = master_cred.key();

    attic::Credentials tcred; // transient credentials
    tcred.set_key(master_key);
    tcred.set_iv(cred.iv());

    std::string encrypted_key;
    attic::crypto::Encrypt(cred.key(), tcred, encrypted_key);

    std::string b64_key;
    attic::crypto::Base64EncodeString(encrypted_key, b64_key);

    std::string b64_dkey;
    attic::crypto::Base64DecodeString(b64_key, b64_dkey);
    // Decrypt
    std::string decrypted_key;
    attic::crypto::Decrypt(b64_dkey, tcred, decrypted_key);

    ASSERT_EQ(cred.key(), decrypted_key);
}

TEST(SCRYPT, ENTER_PASSPHRASE) {
    std::string passphrase("password");

    attic::Credentials cred;                                  
    // Will generate credentials from passphrase
    attic::crypto::GenerateKeyFromPassphrase(passphrase, cred);
    attic::Credentials cred1;
    // Enter passphrase and known salt
    attic::crypto::EnterPassphrase(passphrase, cred.iv(), cred1);   
    ASSERT_EQ(cred.key(), cred1.key());
}

TEST(SCRYPT, ENCODE) {
    std::string input("thisistestinput");
    std::string iv;
    attic::crypto::GenerateNonce(iv); 

    std::string out, out1;
    attic::crypto::ScryptEncode(input, iv, crypto_secretbox_KEYBYTES, out);
    attic::crypto::ScryptEncode(input, iv, crypto_secretbox_KEYBYTES, out1);

    int res =  strcmp(out.c_str(), out1.c_str());
    ASSERT_EQ(res, 0);
}

TEST(FILESYSTEM, RELATIVETO) {
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
TEST(FILESYSTEM, SUBDIRECTORIES) {
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

TEST(AUTHCODE, GENERATE) {
    std::string authcode;
    attic::utils::GenerateRandomString(authcode, 32);
    std::string encoded;
    attic::crypto::Base32EncodeString(authcode, encoded);
    std::string decoded;
    attic::crypto::Base32DecodeString(encoded, decoded);
    ASSERT_EQ(decoded, authcode);

}

/*
TEST(MANIFEST, INSERTFILEINFO) {
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

TEST(MANIFEST, INSERTFILEPOSTID) {
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

TEST(MANIFEST, REMOVEFILEINFO) {
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

TEST(FILEINFO, SERIALIZATION) {
    attic::FileInfo fi;
    fi.set_filepath("<working>/data.pdf");
    fi.set_filename("data.pdf");
    fi.set_file_credentials(attic::crypto::GenerateCredentials());
    fi.set_folder_post_id("folder_post_id");
    fi.set_encrypted_key("encrypted_key");
    fi.set_chunk_count(1);
    fi.set_file_size(124);

    attic::ChunkInfo ci;
    ci.set_chunk_name("chunk name");
    ci.set_checksum("checksum");
    ci.set_plaintext_mac("plaintext mac");
    ci.set_ciphertext_mac("ciphertext mac");
    ci.set_iv("iv");
    ci.set_position(0);



    fi.PushChunkBack(ci);

    std::string chunk_data;
    fi.GetSerializedChunkData(chunk_data);
    std::cout<<" CHUNK DATA : " << chunk_data << std::endl;
}

TEST(CHUNKINFO, SERIALIZATION) {
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

    Json::Value c(Json::nullValue);
    if(attic::jsn::SerializeObject(&ci, c))
        std::cout<<" TRUE " << std::endl;
    else
        std::cout<<" FALSE " << std::endl;

//std::cout<<" STRING : " << s << std::endl;
    attic::jsn::PrintOutJsonValue(&c);

    ASSERT_EQ(output, output2);
}

#define CHUNK_FORMAT 1
void Compose(const std::string& in, const std::string& iv, std::string& out) {
    // Compose the chunk data to its format
    // Format version | iv len | iv | data len | data
    //
    std::cout<<"*****************************************************" << std::endl;
    std::cout<<" compose " << std::endl;
    std::cout<<"\t buffer size : " << in.size() << std::endl;
    unsigned char format = CHUNK_FORMAT;
    std::cout<<"\t FORMAT : " << format << std::endl;
    out.append(format, 1);

    unsigned int iv_size = iv.size();
    unsigned char ivsize[4] = {0};
    ivsize[0] = (iv_size >> 24) & 0xFF;
    ivsize[1] = (iv_size >> 16) & 0xFF;
    ivsize[2] = (iv_size >> 8) & 0xFF;
    ivsize[3] = iv_size & 0xFF;
    std::cout<<"\t IV SIZE : " << iv_size << std::endl;

    out.append(reinterpret_cast<const char*>(ivsize), 4);
    out.append(iv.c_str(), iv_size);

    unsigned int data_size = in.size();
    unsigned char datasize[4] = {0};
    datasize[0] = (data_size >> 24) & 0xFF;
    datasize[1] = (data_size >> 16) & 0xFF;
    datasize[2] = (data_size >> 8) & 0xFF;
    datasize[3] = data_size & 0xFF;
    std::cout<<"\t DATA SIZE : " << data_size << std::endl;
    unsigned int verify = 0;
    verify = (verify << 8) + datasize[0];
    verify = (verify << 8) + datasize[1];
    verify = (verify << 8) + datasize[2];
    verify = (verify << 8) + datasize[3];
    std::cout<<"\t VERIFY SIZE : " << verify << std::endl;

    std::cout<<"\t composed size : " << out.size() << std::endl;
    out.append(reinterpret_cast<const char*>(datasize), 4);


    out.append(in.c_str(), data_size);
    std::cout<<"*****************************************************" << std::endl;
}

void Decompose(const std::string& in, std::string& iv_out, std::string& out) {
    std::cout<<"*****************************************************" << std::endl;
    std::cout<<" decompose " << std::endl;
    std::cout<<"\t buffer size : " << in.size() << std::endl;
    unsigned char format = in[0];
    std::cout<<"\t FORMAT : " << format << std::endl;
    unsigned int offset = 1;
    std::cout<<"\t offset : " << offset << std::endl;
    if(format == CHUNK_FORMAT) {
        unsigned int iv_size = 0;
        iv_size = (iv_size << 8) + in[offset];
        iv_size = (iv_size << 8) + in[offset+1];
        iv_size = (iv_size << 8) + in[offset+2];
        iv_size = (iv_size << 8) + in[offset+3];
        offset+=4;
        std::cout<<"\t offset : " << offset << std::endl;

        std::cout<<"\t IV SIZE : " << iv_size << std::endl;
        iv_out = in.substr(offset, iv_size);
        std::cout<<"\t IV : " << iv_out << std::endl;
        offset+= iv_size;
        std::cout<<"\t offset : " << offset << std::endl;

        unsigned int data_size = 0;
        unsigned char ds_buffer[4];
        ds_buffer[0] = in[offset];
        ds_buffer[1] = in[offset+1];
        ds_buffer[2] = in[offset+2];
        ds_buffer[3] = in[offset+3];

        data_size = (data_size << 8) + ds_buffer[0];
        data_size = (data_size << 8) + ds_buffer[1];
        data_size = (data_size << 8) + ds_buffer[2];
        data_size = (data_size << 8) + ds_buffer[3];
        offset+=4;
        std::cout<<"\t offset : " << offset << std::endl;
        std::cout<<"\t DATA SIZE : " << data_size << std::endl;
        out = in.substr(offset, data_size);
        std::string all = in.substr(offset);
        std::cout<<"\t back end size : " << all.size() << std::endl;
    }
    std::cout<<"*****************************************************" << std::endl;
}

TEST(CHUNK_TRANSFORM, COMPOSE_DECOMPOSE){ 
    std::string payload;
    payload.append("this is my test data,09412");
    //payload.append("\0", 1);
    //payload.append("djfklgjsdg9012345ksajdkfjkasjdgkasjdga");

    /*
    std::ifstream ifs;
    ifs.open("aho.pdf", std::ios::in | std::ios::binary);
    if(ifs.is_open()) {
        ifs.seekg (0, std::ios::end);
        unsigned int size = ifs.tellg();
        ifs.seekg (0, std::ios::beg);
        std::cout<<" file size : " << size << std::endl;

        char* data = new char[size];
        ifs.read(data, size);
        payload.append(data, size);
        delete data;
        data = NULL;
    }
    */

    std::cout<< "payload size : " << payload.size() << std::endl;
    std::string iv("test iv, not a real iv");

    std::string composed;
    Compose(payload, iv, composed);
/*
    std::string b64_encoded;
    attic::crypto::Base64EncodeString(composed, b64_encoded);

    std::string b64_decoded;
    attic::crypto::Base64DecodeString(b64_encoded, b64_decoded);
    */

    std::string iv_out;
    std::string decomposed;
    //Decompose(b64_decoded, iv_out, decomposed);
    Decompose(composed, iv_out, decomposed);
    std::cout<<" decomposed size : " << decomposed.size() << std::endl;

    ASSERT_EQ(iv, iv_out);
    ASSERT_EQ(payload.size(), decomposed.size());
    //ASSERT_EQ(payload, decomposed);
}

TEST(CHUNK, COMPOSE_DECOMPOSE) {
    std::string payload("this is my test data, ksajdkfjkasjdgkasjdga");
    std::string iv("test iv, not a real iv");

    std::string composed;
    unsigned char format = 1;
    composed.append(format, 1);

    char ivsize[4];
    unsigned int n = iv.size();
    ivsize[0] = (n >> 24) & 0xFF;
    ivsize[1] = (n >> 16) & 0xFF;
    ivsize[2] = (n >> 8) & 0xFF;
    ivsize[3] = n & 0xFF;

    composed.append(ivsize, 4);
    /*
    printf("%x %x %x %x\n", (unsigned char)ivsize[0],
                            (unsigned char)ivsize[1],
                            (unsigned char)ivsize[2],
                            (unsigned char)ivsize[3]);
                            */

    composed.append(iv.c_str(), n);

    unsigned int data_size = payload.size();
    char datasize[4] = {0};
    datasize[0] = (data_size >> 24) & 0xFF;
    datasize[1] = (data_size >> 16) & 0xFF;
    datasize[2] = (data_size >> 8) & 0xFF;
    datasize[3] = data_size & 0xFF;

    composed.append(datasize, 4);
    composed.append(payload.c_str(), data_size);

    int offset = 0;
    unsigned char fmt = composed[0];
    offset++;
    unsigned int iv_back = 0;
    std::string eiv = composed.substr(offset, 4);
    for(int i=0; i < sizeof(iv_back); i++) {
        iv_back = (iv_back << 8) + eiv[i];
    }
    offset += 4;
    ASSERT_EQ(iv_back, iv.size());
    ASSERT_EQ(iv, composed.substr(offset, iv_back));
    offset += iv_back;

    unsigned int d_back = 0;
    std::string ddata = composed.substr(offset, 4);
    for(int i=0; i<sizeof(d_back); i++) {
        d_back = (d_back<<8) + ddata[i];
    }
    offset += 4;
    ASSERT_EQ(d_back, payload.size());
    ASSERT_EQ(composed.substr(offset, d_back), payload);
}

/*
TEST(PARAMS, ENCODE) {
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
TEST(FILESYSTEM, SCAN) {
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


TEST(STRTOL, TEST) {
    std::string ts("1370368446");

    time_t tb = strtol(ts.c_str(), NULL, 0);  
    std::cout << " (string) " << ts << std::endl;
    std::cout << " (time_t) " << tb << std::endl;

    time_t e = 1370368446;
    ASSERT_EQ(tb, e);
}

#include <sodium.h>
TEST(SODIUM, SHA256) {
    std::string test("1248i9184591iujwidejgisdjg\0184912gt");
    unsigned char out[crypto_hash_sha512_BYTES];
    sodium_init();
    crypto_hash_sha512(out, reinterpret_cast<const unsigned char*>(test.c_str()),test.size());
    for(int i=0; i<crypto_hash_sha512_BYTES; i++) {
        printf("%02x",(unsigned int) out[i]);
    }

}

unsigned char firstkey[32] = {
     0x1b,0x27,0x55,0x64,0x73,0xe9,0x85,0xd4
         ,0x62,0xcd,0x51,0x19,0x7a,0x9a,0x46,0xc7
         ,0x60,0x09,0x54,0x9e,0xac,0x64,0x74,0xf2
         ,0x06,0xc4,0xee,0x08,0x44,0xf6,0x83,0x89
} ;

unsigned char nonce[16] = {
     0x69,0x69,0x6e,0xe9,0x55,0xb6,0x2b,0x73
         ,0xcd,0x62,0xbd,0xa8,0x75,0xfc,0x73,0xd6
} ;

TEST(SODIUM, aes156STREAM) {
    std::string test("1248i9184591iujwidejgisdjg\0184912gt");
    test += " this is my test string, yep";
    unsigned char c[test.size()];
    crypto_stream_aes256estream_xor(c, 
                                    reinterpret_cast<const unsigned char*>(test.c_str()),
                                    test.size(),
                                    nonce,
                                    firstkey);
    for (int i = 32;i < 163;++i) {
            //printf(",0x%02x",(unsigned int) c[i]);
             //   if (i % 8 == 7) printf("\n");
                  }
      //printf("\n");

    unsigned char p[test.size()]; 
    crypto_stream_aes256estream_xor(p, 
                                    c,
                                    test.size(),
                                    nonce,
                                    firstkey);

    std::cout<<" original : " << test << std::endl;
    std::string output;
    output.append(reinterpret_cast<const char*>(p), test.size());
    std::cout<<" decrypted : " << output << std::endl;

    ASSERT_EQ(test, output);
}

TEST(SODIUM, SECRET_BOX) {
    char pad[32]={0};
    
    std::string msg;
    //msg.append(pad, 32);
    msg.append(32, 0);
    msg += "this is my test message";

    std::cout<<msg.size()<< std::endl;
    std::cout<<" original msg : " << msg << std::endl;
    std::cout<<msg.size()<< std::endl;
    unsigned char ciphertext[msg.size()];
    unsigned char key[crypto_secretbox_KEYBYTES];
    randombytes(key, crypto_secretbox_KEYBYTES);
    unsigned char iv[crypto_secretbox_NONCEBYTES];
    randombytes(iv, crypto_secretbox_NONCEBYTES);

    int result = crypto_secretbox(ciphertext, 
                                  reinterpret_cast<const unsigned char*>(msg.c_str()), 
                                  msg.size(),
                                  iv, 
                                  key);

    unsigned char m2[msg.size()];
    std::string f;
    if (crypto_secretbox_open(m2,
                              ciphertext,
                              msg.size(),
                              iv,
                              key) == 0) {
        f.append(reinterpret_cast<const char*>(m2), msg.size());
        std::cout<<" opened the box : " << f << std::endl;
        std::cout<<f.size() << std::endl;
        std::cout<< f.substr(32) << std::endl;
    }
    else {
        std::cout<<" ciphertext fails verification" << std::endl;
    }
    ASSERT_EQ(f, msg);
}

TEST(SODIUM, CRYPTO_GENERICHASH) {
    std::string data("The quick brown fox jumps over the lazy dog");
    crypto_generichash_state st; // state
    unsigned char hash[64];
    crypto_generichash_init(&st, NULL, NULL, 64);
    for(int i=0; i<data.size(); i++) {
        unsigned char a[] = {static_cast<unsigned char>(data[i])};
        crypto_generichash_update(&st, a, 1);
    }
    crypto_generichash_final(&st, hash, 64);

    std::string final_hash;
    for(int i=0; i<64; i++)  {
        char b[20] = {'\0'};
        snprintf(b, 20, "%02x",(unsigned int) hash[i]);
        final_hash += b;
    }
    std::string upper;
    for(unsigned int i=0; i<final_hash.size(); i++) 
        upper += toupper(final_hash[i]);
    //std::cout<<" hash : " << upper << std::endl;

    // test vector from wikipedia
    // http://en.wikipedia.org/wiki/BLAKE_%28hash_function%29
    std::string test_answer = "A8ADD4BDDDFD93E4877D2746E62817B116364A1FA7BC148D95090BC7333B3673F82401CF7AA2E4CB1ECD90296E3F14CB5413F8ED77BE73045B13914CDCD6A918";
    ASSERT_EQ(upper, test_answer);
}

TEST(SODIUM, FILEROLLER) {
    /*
    attic::FileRoller fr("miami.mp4");
    std::string hash;
    if(fr.Digest(hash))
        std::cout<<"hash out : " << hash << std::endl;
    else
        std::cout<<" failed to digest " << std::endl;
        */
}




int main (int argc, char* argv[]) {
   int status = 0;
    // Init gtestframework
    testing::InitGoogleTest(&argc, argv);
       
    // All tests are setup, run them
    status = RUN_ALL_TESTS();

    return status;
}
