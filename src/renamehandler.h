#ifndef RENAMEHANDLER_H_
#define RENAMEHANDLER_H_
#pragma once

#include <vector>
#include <string>

namespace attic {
    
class FileManager;

class RenameHandler {
public:
    RenameHandler(FileManager* fi);
    ~RenameHandler();

    bool RenameFileLocalCache(const std::string& old_filepath, const std::string& new_name);
    bool RenameFileLocalCache(const std::vector<std::string>& alias_list, 
                              const std::string& new_filepath, 
                              const std::string& file_post_id);
private:
    FileManager* file_manager_;
};

} //namespace
#endif

