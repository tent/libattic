#include "folderpost.h"

#include "constants.h"
#include "errorcodes.h"


FolderPost::FolderPost()
{
    SetPostType(cnst::g_szFolderPostType);
    SetPublic(false);
}

FolderPost::FolderPost(const Folder& folder)
{
    SetPostType(cnst::g_szFolderPostType);
    SetPublic(false);

    m_Folder = folder;
}

FolderPost::~FolderPost()
{

}

void FolderPost::Serialize(Json::Value& root)
{
    Json::Value folder(Json::objectValue);
    jsn::SerializeObject(&m_Folder, folder);

    SetContent("children", folder);

    Post::Serialize(root);
}

void FolderPost::Deserialize(Json::Value& root)
{
    Post::Deserialize(root);
    Json::Value folder;
    GetContent("children", folder);
    jsn::DeserializeObject(&m_Folder, folder);
}

int FolderPost::PushBackFolderPost(FolderPost& post)
{
    int status = ret::A_OK;




    return status;
}
