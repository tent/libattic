#include "folderpost.h"

#include "constants.h"
#include "errorcodes.h"

FolderPost::FolderPost()
{
    SetPostType(cnst::g_szFolderPostType);
}

FolderPost::~FolderPost()
{

}

void FolderPost::Serialize(Json::Value& root)
{

}

void FolderPost::Deserialize(Json::Value& root)
{

}

int FolderPost::PushBackFolderPost(FolderPost& post)
{
    int status = ret::A_OK;




    return status;
}
