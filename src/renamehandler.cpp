#include "renamehandler.h"

#include "filemanager.h"

namespace attic { 

RenameHandler::RenameHandler(FileManager* fi) {
    file_manager_ = fi;
}

RenameHandler::~RenameHandler() {
    file_manager_ = NULL;
}

bool RenameHandler::RenameFileLocalCache(const std::string& old_filepath, 
                                         const std::string& new_name) {

    return false;
}

bool RenameHandler::RenameFileLocalCache(const std::vector<std::string>& alias_list, 
                                         const std::string& new_filepath, 
                                         const std::string& file_post_id) {

    return false;
}

}//namespace
