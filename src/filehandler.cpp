#include "filehandler.h"

#include "filesystem.h"
#include "crypto.h"
#include "credentials.h"
#include "utils.h"

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

// Note* there are 3 keys that file info objects are queried by, this only sets two of them,
// if the third is not set, a new file with the same path and folderid can be created for a 
// duplicate entry
bool FileHandler::CreateNewFile(const std::string& filepath, 
                                const std::string& master_key,
                                FileInfo& out) {
    if(!file_manager_->DoesFileExist(filepath)) {
        std::string folderpath;
        if(fs::GetParentPath(filepath, folderpath) == ret::A_OK) {
            std::string folderid;
            if(file_manager_->GetFolderPostId(folderpath, folderid)) {
                std::string aliased, filename;
                utils::ExtractFileName(filepath, filename);
                file_manager_->GetAliasedFilepath(filepath, aliased);
                std::cout<<" folder path : " << folderpath << std::endl;
                std::cout<<" folder id : " << folderid << std::endl;
                std::cout<<" aliased : " << aliased << std::endl;
                out.set_filepath(aliased);
                out.set_folder_post_id(folderid);
                out.set_filename(filename);
                Credentials file_cred;
                while(file_cred.key_empty())
                    crypto::GenerateCredentials(file_cred);
                out.set_file_credentials(file_cred);
                EncryptFileKey(out, master_key);
                file_manager_->InsertToManifest(&out);
                return true;
            }
        }
    }
    return false;
}

bool FileHandler::UpdateFilePostId(const std::string& filepath, const std::string& post_id) {
    return file_manager_->SetFilePostId(filepath, post_id);
}

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
        Credentials tcred; 
        tcred.set_key(master_key);                 // master key
        tcred.set_iv(file_iv);                     // file specific iv
        // Encryption call
        std::string encrypted_key;
        crypto::EncryptStringGCM(file_key, tcred, encrypted_key);
        return true;
    }
    return false;
}



}//namespace
