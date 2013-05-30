#include "filehandler.h"

#include "filesystem.h"
#include "crypto.h"
#include "credentials.h"
#include "utils.h"
#include "logutils.h"

namespace attic { 

FileHandler::FileHandler(FileManager* fm) {
    file_manager_ = fm;
}

FileHandler::~FileHandler() {}

bool FileHandler::DoesFileExist(const std::string& filepath) {
    return file_manager_->DoesFileExist(filepath);
}
bool FileHandler::RetrieveFileInfo(const std::string& filepath, FileInfo& out) {
    return file_manager_->GetFileInfo(filepath, out);
}

bool FileHandler::RetrieveFileInfoById(const std::string& post_id, FileInfo& out) {
    return file_manager_->GetFileInfoByPostId(post_id, out);
}

// Note* there are 3 keys that file info objects are queried by, this only sets two of them,
// if the third is not set, a new file with the same path and folderid can be created for a 
// duplicate entry
bool FileHandler::CreateNewFile(const std::string& filepath, 
                                const std::string& master_key,
                                FileInfo& out) {
    if(master_key.empty()) {
        std::ostringstream err;
        err << " Invalid master key : "<< master_key << std::endl;
        log::LogString("0010-12851-", err.str());
        return false;
    }

    if(!file_manager_->DoesFileExist(filepath)) {
        std::string folderpath;
        if(fs::GetParentPath(filepath, folderpath) == ret::A_OK) {
            std::string folderid;
            std::cout<< " checking folderpath : " << folderpath << std::endl;
            utils::CheckUrlAndRemoveTrailingSlash(folderpath);
            std::cout<< " checking folderpath : " << folderpath << std::endl;
            if(file_manager_->GetFolderPostId(folderpath, folderid)) {
                std::string aliased, filename;
                utils::ExtractFileName(filepath, filename);
                file_manager_->GetAliasedFilepath(filepath, aliased);
                std::cout<<" folder path : " << folderpath << std::endl;
                std::cout<<" folder id : " << folderid << std::endl;
                std::cout<<" aliased : " << aliased << std::endl;
                out.set_filename(filename);             // set filename
                out.set_filepath(aliased);              // set filepath
                out.set_folder_post_id(folderid);       // set folder id
                out.set_chunk_count(0);
                Credentials file_cred;
                while(file_cred.key_empty())
                    crypto::GenerateCredentials(file_cred);
                out.set_file_credentials(file_cred);    // set credentials
                EncryptFileKey(out, master_key); // sets encryted file key on file info
                // set filesize
                out.set_file_size(utils::CheckFilesize(filepath));
                return file_manager_->InsertToManifest(&out);
            }
        }
    }
    return false;
}

bool FileHandler::GetCanonicalFilepath(const std::string& filepath, std::string& out) {
    return file_manager_->GetCanonicalFilepath(filepath, out);
}

bool FileHandler::UpdateFileInfo(FileInfo& fi) {
    return file_manager_->InsertToManifest(&fi);
}

bool FileHandler::UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath) {
    return file_manager_->SetNewFilepath(old_filepath, new_filepath);
}

bool FileHandler::UpdateFilePostId(const std::string& filepath, const std::string& post_id) {
    return file_manager_->SetFilePostId(filepath, post_id);
}

bool FileHandler::UpdateChunkCount(const std::string& filepath, const std::string& count) {
    return file_manager_->SetFileChunkCount(filepath, count);
}

bool FileHandler::UpdateFileSize(const std::string& filepath, const std::string& size) {
    // TODO:: this
    std::cout<< " fh IMPLEMENT THIS " << std::endl;
    return false;
}

bool FileHandler::UpdateChunkMap(const std::string& filepath, FileInfo::ChunkMap& map) {
    return file_manager_->SetFileChunks(filepath, map);
}

bool FileHandler::UpdatePostVersion(const std::string& filepath, const std::string& version) {
    return file_manager_->SetFileVersion(filepath, version);
}

bool FileHandler::UpdateFolderEntry(FolderPost& fp) {
    return file_manager_->UpdateFolderEntry(fp.folder().folderpath(), fp.id());
}

void FileHandler::DeserializeIntoFileInfo(FilePost& fp, FileInfo& out) {
    out.set_filename(fp.name());
    out.set_filepath(fp.relative_path());
    out.set_encrypted_key(fp.key_data());
    out.set_file_credentials_iv(fp.iv_data());
    out.set_post_id(fp.id());
    out.set_folder_post_id(fp.folder_post());
    out.set_post_version(fp.version().id());
    out.set_file_size(fp.file_size());
}

bool FileHandler::GetTemporaryFilepath(FileInfo& fi, std::string& path_out) {
    std::string temp_path = file_manager_->temp_directory();
    if(fs::CheckFilepathExists(temp_path)) {
        utils::CheckUrlAndAppendTrailingSlash(temp_path);
        std::string randstr;
        utils::GenerateRandomString(randstr, 16);
        temp_path += fi.filename() + "_" + randstr;
        path_out = temp_path;
        return true;
    }
    return false;
}

// TODO :: reconsider the relevance of these crypto operations in this class,
//         perhaps move then to their own class? For now here is fine
bool FileHandler::EncryptFileKey(const std::string& filepath, 
                                 const std::string& file_key,
                                 const std::string& master_key) {
    if(!master_key.empty()) {
        FileInfo fi;
        if(RetrieveFileInfo(filepath, fi))
            return EncryptFileKey(fi, master_key);
    }
    return false;
}

bool FileHandler::EncryptFileKey(FileInfo& fi, const std::string& master_key) {

    if(!master_key.empty()) {
        std::string encrypted_key;
        if(EncryptFileKey(fi.file_credentials_key(),
                          fi.file_credentials_iv(), 
                          master_key, 
                          encrypted_key)) {
            fi.set_encrypted_key(encrypted_key);
            return true;
        }
    }
    return false;
}

bool FileHandler::EncryptFileKey(const std::string& file_key,
                                 const std::string& file_iv,
                                 const std::string& master_key,
                                 std::string& encrypted_out) {
    if(!master_key.empty() && !file_key.empty() && !file_iv.empty()) {
        Credentials tcred;                         // Create transient credentials
        tcred.set_key(master_key);                 // master key
        tcred.set_iv(file_iv);                     // file specific iv
        // Encryption call
        crypto::EncryptStringGCM(file_key, tcred, encrypted_out);
        return true;
    }
    return false;
}

// Extract File credentials directly from a filepost
bool FileHandler::ExtractFileCredetials(const FilePost& fp,
                                        const std::string& master_key,
                                        Credentials& out) {
    if(!master_key.empty()) {
        Credentials tcred;                         // Create transient credentials
        tcred.set_key(master_key);                 // master key
        tcred.set_iv(fp.iv_data());                // file specific iv

        std::string decrypted_key;
        if(crypto::DecryptStringGCM(fp.key_data(), tcred, decrypted_key) == ret::A_OK) {
            out.set_key(decrypted_key);
            out.set_iv(fp.iv_data());
            return true;
        }
    }
    return false;
}



}//namespace
