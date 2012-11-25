

#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_
#pragma once

namespace ret
{
    enum eCode
    {
        A_OK = 0,
        A_FAIL,
        A_FAIL_OPEN,
        A_FAIL_ENCRYPT,
        A_FAIL_DECRYPT,
        A_FAIL_COMPRESS,
        A_FAIL_DECOMPRESS,
        A_FAIL_INVALID_PTR,
        A_FAIL_VERIFY_CHUNKS,
        A_FAIL_WRITE_CHUNK,
        A_FAIL_ATTEMPT_DOUBLE_FREE,
        A_FAIL_TO_SERIALIZE_OBJECT,
        A_FAIL_TO_DESERIALIZE_OBJECT,
        A_FAIL_INVALID_CSTR,
        A_FAIL_TO_LOAD_FILE,
        A_FAIL_FILE_NOT_IN_MANIFEST,
        A_LIB_FAIL_INVALID_APP_INSTANCE,
        A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE

    };

};



#endif

