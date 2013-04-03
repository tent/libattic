#include "postfilemetadatastrategy.h"

#include "utils.h"
#include "netlib.h"
#include "postutils.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "logutils.h"

namespace attic { 

PostFileMetadataStrategy::PostFileMetadataStrategy() {}
PostFileMetadataStrategy::~PostFileMetadataStrategy() {}

int PostFileMetadataStrategy::Execute(FileManager* pFileManager,
                                      CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);

    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filepath = GetConfigValue("filepath");

    FileInfo* fi = RetrieveFileInfo(filepath);
    if(fi) {
        int trycount = 0;
        for(status = SendFilePost(fi, filepath); status != ret::A_OK; trycount++) {
            status = SendFilePost(fi, filepath);
            std::cout<<" RETRYING .................................." << std::endl;
            if(trycount > 2)
                break;
        }
    }
    else { 
        std::cout<<" INVALID FILE INFO " << std::endl;
        status = ret::A_FAIL_INVALID_FILE_INFO;
    }
    
    return status;
}

int PostFileMetadataStrategy::SendFilePost( FileInfo* fi, const std::string& filepath) {
    std::cout<<" send attic post filepath : " << filepath << std::endl;
    int status = ret::A_OK;
    std::cout<<" SEND ATTIC POST " << std::endl;
    // Create Attic Post
    if(!fi)
        std::cout<<"invalid file info"<<std::endl;

    // Check for existing post
    std::string posturl;
    std::string postid = fi->post_id();

    std::string relative_path = fi->filepath();
    std::cout<<" INSERTING RELATIVE PATH TO POST : " << relative_path << std::endl;
    bool post = true;
    Response response;
    if(postid.empty()) {
        posturl = posts_feed_;
        // New Post
        std::cout<< " POST URL : " << posturl << std::endl;
        unsigned int size = utils::CheckFilesize(filepath);
        FilePost p;
        postutils::InitializeFilePost(fi, p, false);
        
        std::string postBuffer;
        jsn::SerializeObject(&p, postBuffer);

        std::cout<<"\n\n Attic Post Buffer : " << postBuffer << std::endl;

        status = netlib::HttpPost(posturl,
                                  p.type(),
                                  NULL,
                                  postBuffer,
                                  &access_token_,
                                  response );
    }
    else {
        utils::FindAndReplace(post_path_, "{post}", postid, posturl);
        post = false;
        std::cout<< " PUT URL : " << posturl << std::endl;
        
        unsigned int size = utils::CheckFilesize(filepath);
        FilePost p;

        postutils::InitializeFilePost(fi, p, false);
        
        std::string postBuffer;
        jsn::SerializeObject(&p, postBuffer);

        status = netlib::HttpPut(posturl,
                                 p.type(),
                                 NULL,
                                 postBuffer,
                                 &access_token_,
                                 response );
   }

    // Handle Response
    if(response.code == 200) {
        FilePost p;
        jsn::DeserializeObject(&p, response.body);

        std::string postid = p.id();
        if(!postid.empty()) {
            fi->set_post_id(postid); 
            if(post){
                std::string fi_filepath = fi->filepath();
                //fi->SetPostVersion(p.GetVersion()); // TODO update this in the manifest

                file_manager_->SetFilePostId(fi_filepath, postid);
                //char szVer[256] = {'\0'};
                //snprintf(szVer, 256, "%d", p.GetVersion());
                //file_manager_->SetFileVersion(fi_filepath, std::string(szVer));
                // set post version
                // Send Folder Post
                std::cout<<" sending folder post to filepath : " << fi_filepath << std::endl;
                //SendFolderPost(fi);
            }
        }
        else{
            std::cout<<" EMPTY POST ID ON RETURN " << std::endl;
        }
    }
    else {
        log::LogHttpResponse("MOP129409", response);
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

FileInfo* PostFileMetadataStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = file_manager_->GetFileInfo(filepath);
    if(!fi)
        fi = file_manager_->CreateFileInfo();
    return fi;
}

}//namespace
