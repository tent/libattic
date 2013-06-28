#include "compression.h"

namespace attic {

#include <miniz.c> // have to pull in miniz.c into attic namespace to avoid unexpected namespace errors
int Compress::CompressString(const std::string& in, std::string& out) {
    int status = ret::A_OK;
    unsigned long src_len = (unsigned long)in.size();
    unsigned long cmp_len = compressBound(src_len);
    unsigned long uncomp_len = src_len;
    unsigned char* pCmp = new unsigned char[cmp_len];
    // Compress the string.
    int cmp_status = mz_compress(pCmp, &cmp_len, (const unsigned char *)in.c_str(), src_len);
    if (cmp_status == Z_OK) {
        printf("Compressed from %u to %u bytes\n", (mz_uint32)src_len, (mz_uint32)cmp_len);
        out.append((const char*)pCmp, (unsigned int)cmp_len);
        std::cout<<" out size : " << out.size() << std::endl;
    }
    else {
        printf("compress() failed!\n");
        status = ret::A_FAIL_COMPRESS;
    }
 
    if(pCmp) {
        delete[] pCmp;
        pCmp = NULL;
    }
    return status;
}

int Compress::DecompressString(const std::string& in, 
                               unsigned int expected_size,
                               std::string& out) {
    int status = ret::A_OK;
    unsigned long uncmp_len = (uLong)expected_size; // uncompressed size
    unsigned char* pUncomp = new unsigned char[expected_size]; // destination
    int cmp_status = uncompress(pUncomp, 
                                &uncmp_len, 
                                (const unsigned char*)in.c_str(), 
                                (unsigned long)in.size());
    if(cmp_status == Z_OK) {
        out.append((const char*)pUncomp, expected_size);
    }
    else {
        std::cout<<" failed to decompress " << std::endl;
        std::cout<<" cmp status : " << cmp_status << std::endl;
        status = ret::A_FAIL_DECOMPRESS;
    }

    if(pUncomp) {
        delete[] pUncomp;
        pUncomp = NULL;
    }

    return status;
}

int Compress::AppendFileToZipArchive(const std::string& directory_path,
                                     const std::string& archive_name,
                                     const std::string& filepath) {
    int status = ret::A_OK;

    return status;
}


bool Archive::InitArchive(const std::string& archivepath) {

}

bool Archive::AddFile(const std::string& filepath) {

}

bool Archive::AddFromMemory(const std::string& filename, const std::string& buffer) {

}



}//namespace

