#include "folderpost.h"

#include "constants.h"
#include "errorcodes.h"

FolderPost::FolderPost() {
    set_type(cnst::g_attic_folder_type);
    set_public(false);
}

FolderPost::FolderPost(const Folder& folder) {
    FolderPost::FolderPost();
    folder_ = folder;
}

FolderPost::~FolderPost() {}

void FolderPost::Serialize(Json::Value& root) {
    Json::Value folder(Json::objectValue);
    jsn::SerializeObject(&folder_, folder);

    set_content("children", folder);

    Post::Serialize(root);
}

void FolderPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);
    Json::Value folder;
    get_content("children", folder);
    jsn::DeserializeObject(&folder_, folder);
}

