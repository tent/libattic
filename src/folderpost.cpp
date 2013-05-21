#include "folderpost.h"

#include "constants.h"
#include "errorcodes.h"

namespace attic { 

FolderPost::FolderPost() {
    set_type(cnst::g_attic_folder_type);
}

FolderPost::FolderPost(const Folder& folder) {
    set_type(cnst::g_attic_folder_type);
    folder_ = folder;
}

FolderPost::~FolderPost() {}

void FolderPost::Serialize(Json::Value& root) {
    Json::Value folder(Json::objectValue);
    folder["folderpath"] = folder_.folderpath();
    folder["parent_post"] = folder_.parent_post_id();
    set_content("folder", folder);

    Post::Serialize(root);
}

void FolderPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    Json::Value folder(Json::objectValue);
    get_content("folder", folder);
    folder_.set_folderpath(folder.get("folderpath", "").asString());
    folder_.set_parent_post_id(folder.get("parent_post", "").asString());
    folder_.set_folder_post_id(id());
}


}//namespace
