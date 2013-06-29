#include "compression.h"
#include "utils.h"
#include "filesystem.h"

namespace attic {

#include <miniz.c> // have to pull in miniz.c into attic namespace to avoid unexpected namespace errors

#if defined(__GNUC__)
#define _FILE_OFFSET_BITS 64
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long long uint64;
typedef unsigned int uint;

static static bool ensure_file_exists_and_is_readable(const char *pFilename) {
    FILE *p = fopen(pFilename, "rb");
    if (!p) return false;
    //_fseeki64(p, 0, SEEK_END);
    fseeko64(p, 0, SEEK_END);
    //uint64 src_file_size = _ftelli64(p);
    uint64 src_file_size = ftello64(p);
    //_fseeki64(p, 0, SEEK_SET);
    fseeko64(p, 0, SEEK_SET);

    if (src_file_size) {
        char buf[1];
        if (fread(buf, 1, 1, p) != 1) {
            fclose(p);
            return false;
        }
    }
    fclose(p);
    return true;
}

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


Archive::Archive(){}
Archive::~Archive(){}

bool Archive::DetermineRelativePath(const std::string& root_dir, 
                                    const std::string& filepath,
                                    std::string& out) {
    bool ret = false;
    size_t pos = filepath.find(root_dir);
    if(pos != std::string::npos) {
        out = filepath.substr(pos + root_dir.size());
        utils::RemoveBeginningSlash(out);
        ret = true;
    }
    else {
        // just stick in in the top directory
        pos = filepath.rfind("/");
        if(pos != std::string::npos)
            out = filepath.substr(pos+1);
        else
            out = filepath;
    }

    return ret;
}

bool Archive::AddFile(const std::string& archive_path,
                      const std::string& root_dir,
                      const std::string& filepath) {
    bool ret = false;
    if(fs::CheckFilepathExists(filepath)) {
        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        if ((rand() % 100) >= 10)
            zip.m_file_offset_alignment = 1 << (rand() & 15);
        if (mz_zip_writer_init_file(&zip, archive_path.c_str(), 65537)) {
            mz_bool success = MZ_TRUE;
            if(ensure_file_exists_and_is_readable(filepath.c_str())) {
                // determine relative path within the archive
                std::string relative_path;
                DetermineRelativePath(root_dir, filepath, relative_path);
                const char *pTestComment = "no comment";
                success &= mz_zip_writer_add_file(&zip, 
                                              relative_path.c_str(),// where in the archive directory is is
                                              filepath.c_str(),  // actual file location
                                              pTestComment, 
                                              (uint16)strlen(pTestComment), 1);
            }
            if (!success) {
                mz_zip_writer_end(&zip);
                remove(archive_path.c_str());
                std::cout<< "failed to create zip archive" << std::endl;
                return false;
            }
            if (!mz_zip_writer_finalize_archive(&zip)) {
                mz_zip_writer_end(&zip);
                remove(archive_path.c_str());
                std::cout<< "failed to create zip archive" << std::endl;
                return false;
            }
            mz_zip_writer_end(&zip);
            ret = true;
        }
        else  {
            std::cout<<" fialed creating : " << archive_path << " archive " << std::endl;
        }
    }
    else {
        std::cout<<" invalid filepath " << std::endl;
    }
    return ret;
}

bool Archive::AddFromMemory(const std::string& archive_path,
                            const std::string& filename, 
                            const std::string& buffer) {
    bool ret = false;
    return true;
}



}//namespace

