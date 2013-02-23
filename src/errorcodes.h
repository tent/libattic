#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_
#pragma once

#include <stdio.h>
namespace ret
{
    enum eCode
    {
        A_OK = 0,
        A_FAIL,
        A_FAIL_OPEN_FILE,
        A_FAIL_ENCRYPT,
        A_FAIL_DECRYPT,
        A_FAIL_HMAC, // 5
        A_FAIL_HMAC_VERIFY,
        A_FAIL_COMPRESS,   
        A_FAIL_DECOMPRESS,
        A_FAIL_VERIFY_CHUNKS,
        A_FAIL_WRITE_CHUNK, // 10 
        A_FAIL_TO_SERIALIZE_OBJECT, 
        A_FAIL_TO_DESERIALIZE_OBJECT,
        A_FAIL_INVALID_CSTR,
        A_FAIL_TO_LOAD_FILE,
        A_FAIL_TO_WRITE_OUT_MANIFEST, // 15
        A_FAIL_FILE_NOT_IN_MANIFEST,
        A_FAIL_TO_QUERY_MANIFEST,
        A_FAIL_COULD_NOT_FIND_POSTS,
        A_FAIL_JSON_PARSE,
        A_FAIL_CURL_PERF, // 20
        A_FAIL_SCRYPT_INVALID_SALT_SIZE,
        A_FAIL_EMPTY_PASSPHRASE,
        A_FAIL_REGISTER_PASSPHRASE,
        A_FAIL_ENTITY_ALREADY_LOADED,
        A_FAIL_INVALID_PHRASE_TOKEN, // 25
        A_FAIL_SENTINEL_MISMATCH,
        A_FAIL_NEED_ENTER_PASSPHRASE,
        A_FAIL_NON_200,
        A_FAIL_INVALID_MASTERKEY,
        A_FAIL_KEYSIZE_MISMATCH, // 30
        A_FAIL_IVSIZE_MISMATCH,
        A_FAIL_INVALID_APP_INSTANCE,
        A_FAIL_INVALID_FILEMANAGER_INSTANCE,
        A_FAIL_INVALID_CONNECTIONMANAGER_INSTANCE,
        A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE, // 35
        A_FAIL_INVALID_ENTITYMANAGER_INSTANCE,
        A_FAIL_INVALID_UPLOADMANAGER_INSTANCE,
        A_FAIL_ATTEMPT_TO_REINIT,
        A_FAIL_DUPLICATE_ENTRY,
        A_FAIL_RUNNING_SINGLE_INSTANCE, // 40
        A_FAIL_CREATE_THREAD, 
        A_FAIL_NO_CREDENTIALS,
        A_FAIL_INVALID_PTR, // to be used sparingly as a placeholder please
        A_FAIL_ATTEMPT_DOUBLE_FREE, 
        A_FAIL_HEX_ENCODE, // 45
        A_FAIL_HEX_DECODE, 
        A_FAIL_INVALID_FILE_KEY,
        A_FAIL_LOAD_APP_DATA,
        A_FAIL_TIMED_OUT,
        A_FAIL_LIB_INIT, // 50
        A_FAIL_SUBSYSTEM_NOT_INITIALIZED,
        A_FAIL_EMPTY_STRING,
        A_FAIL_TCP_ENDPOINT_NOT_FOUND,
        A_FAIL_SSL_HANDSHAKE,
        A_FAIL_EMPTY_ATTACHMENTS, // 55
        A_FAIL_OTHER     
    };

    static std::string ErrorToString(int code)
    {
        char buf[256] = {'\0'};
        sprintf(buf, "%d", code);
        std::string codestr;
        codestr.append(buf);
        return codestr;
    }
};

#endif

