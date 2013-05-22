#include "filehandler.h"

#include "filesystem.h"

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
                std::cout<<" folder path : " << folderpath << std::endl;
                std::cout<<" folder id : " << folderid << std::endl;
            }
        }
        return true;
    }
    return false;
}


