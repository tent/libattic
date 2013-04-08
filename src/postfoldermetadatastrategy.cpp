#include "postfoldermetadatastrategy.h"

#include "utils.h"
#include "netlib.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "postutils.h"
#include "folderpost.h"
#include "logutils.h"

namespace attic { 

PostFolderMetadataStrategy::PostFolderMetadataStrategy() {}
PostFolderMetadataStrategy::~PostFolderMetadataStrategy() {}

int PostFolderMetadataStrategy::Execute(FileManager* pFileManager,
                                        CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);

    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filepath = GetConfigValue("filepath");

    FileInfo* fi = RetrieveFileInfo(filepath);
    Response response;
    status = SendFolderPost(fi, response);

    return status;
}

int PostFolderMetadataStrategy::SendFolderPost(const FileInfo* fi, Response& out) {
    int status = ret::A_OK;

    std::string filepath = fi->filepath();
    std::string filename = fi->filename();
    std::string parent_relative;
    int pos = filepath.find(filename);
    if(pos == std::string::npos) { 
        std::cout<<"MALFORMED FILEPATH " << filepath << std::endl;
        return -1;
    }
    parent_relative = filepath.substr(0, pos-1);

    Folder folder;
    std::cout<<"PARENT RELATIVE : " << parent_relative << std::endl;
    if(file_manager_->GetFolderInfo(parent_relative, folder)) {
        // serialize and send
        FolderPost p(folder);
        std::string posturl;
        std::string postid;
        folder.GetPostID(postid);
        std::cout<<" FOLDER POST : " << postid << std::endl;

        std::string postBuffer;


        std::cout<<" POST BUFFER : \n" << postBuffer << std::endl;

        Response response;

        bool bPost = true;
        if(postid.empty()) { // POST
            posturl = posts_feed_;
            std::cout<< "FOLDER POST URL : " << posturl << std::endl;
            std::cout<< " type : " << p.type() << std::endl;
            jsn::SerializeObject(&p, postBuffer);
            status = netlib::HttpPost( posturl,
                                       p.type(),
                                       NULL,
                                       postBuffer,
                                       &access_token_,
                                       response);

            out = response;
        }
        else { // PUT
            bPost = false;
            utils::FindAndReplace(post_path_, "{post}", postid, posturl);
            std::cout<<"FOLDER PUT URL : " << posturl << std::endl;
            // Get old version
            Response resp;
            netlib::HttpGet(posturl, NULL, &access_token_, resp);
            if(resp.code == 200) {
                FolderPost old_post;
                jsn::DeserializeObject(&old_post, resp.body);
                Parent parent;
                parent.version = old_post.version()->id;
                p.PushBackParent(parent);
               
                jsn::SerializeObject(&p, postBuffer);
                status = netlib::HttpPut(posturl,
                                         p.type(),
                                         NULL,
                                         postBuffer,
                                         &access_token_,
                                         response );
            }
            out = response;
        }

        std::cout<<" FOLDER POST RESPONSE CODE : " << response.code << std::endl;
        std::cout<<" FOLDER POST RESPONSE BODY : " << response.body << std::endl;

        if(response.code == 200) {
            FolderPost p;
            jsn::DeserializeObject(&p, response.body);

            std::string buffer;
            jsn::SerializeObject(&p, buffer);
            std::cout<<" retreived body : " << buffer << std::endl;
            
            std::string postid = p.id();

            if(!postid.empty()) {
                if(bPost) {
                    file_manager_->SetFolderPostId(parent_relative, postid);
                }
            }
        }
        else {
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("591230MA", response);
        }
    }
    else {
        std::cout<<" FOLDER NOT IN MANIFEST " << std::endl;
    }

    return status;
}

FileInfo* PostFolderMetadataStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = file_manager_->GetFileInfo(filepath);
    if(!fi)
        fi = file_manager_->CreateFileInfo();
    return fi;
}

}// namespace
