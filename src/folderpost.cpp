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

    Json::Value folder_content(Json::objectValue);
    Json::Value aliases;
    SerializePastAliases(aliases);
    folder_content["past_aliases"] = aliases;
    set_content("folder_content", folder_content);

    Post::Serialize(root);
}

void FolderPost::SerializePastAliases(Json::Value& val) {
    jsn::SerializeVector(past_aliases_, val);
}

void FolderPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    Json::Value folder(Json::objectValue);
    get_content("folder", folder);
    folder_.set_folderpath(folder.get("folderpath", "").asString());
    folder_.set_parent_post_id(folder.get("parent_post", "").asString());
    folder_.set_folder_post_id(id());

    Json::Value folder_content;
    get_content("folder_content", folder_content);
    DeserializePastAliases(folder_content["past_aliases"]);
}

void FolderPost::DeserializePastAliases(Json::Value& val) {
    jsn::DeserializeIntoVector(val, past_aliases_);
    for(int i=0; i<past_aliases_.size();i++)
        std::cout<<past_aliases_[i]<<std::endl;
}

}//namespace
