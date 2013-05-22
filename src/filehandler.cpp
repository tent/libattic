#include "filehandler.h"

#include "filesystem.h"
#include "crypto.h"
#include "credentials.h"

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

bool FileHandler::CreateNewFile(const std::string& filepath, FileInfo& out) {
    if(!file_manager_->DoesFileExist(filepath)) {
        std::string folderpath;
        if(fs::GetParentPath(filepath, folderpath) == ret::A_OK) {
            std::string folderid;
            if(file_manager_->GetFolderPostId(folderpath, folderid)) {
                std::string aliased;
                file_manager_->GetAliasedFilepath(filepath, aliased);
                std::cout<<" folder path : " << folderpath << std::endl;
                std::cout<<" folder id : " << folderid << std::endl;
                std::cout<<" aliased : " << aliased << std::endl;
                out.set_filepath(aliased);
                out.set_folder_post_id(folderid);
                Credentials file_cred;
                while(file_cred.key_empty())
                    crypto::GenerateCredentials(file_cred);
                out.set_file_credentials(file_cred);
                return true;
            }
        }
    }
    return false;
}

bool FileHandler::UpdateFilePostId(const std::string& filepath, const std::string& post_id) {
    return file_manager_->SetFilePostId(filepath, post_id);
}

}//namespace
