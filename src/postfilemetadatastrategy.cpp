#include "postfilemetadatastrategy.h"

#include "utils.h"
#include "netlib.h"
#include "postutils.h"
#include "filemanager.h"
#include "credentialsmanager.h"

PostFileMetadataStrategy::PostFileMetadataStrategy() {}
PostFileMetadataStrategy::~PostFileMetadataStrategy() {}

int PostFileMetadataStrategy::Execute(FileManager* pFileManager,
                                       CredentialsManager* pCredentialsManager,
                                       Response& out)
{
    int status = ret::A_OK;
    m_pFileManager = pFileManager;
    m_pCredentialsManager = pCredentialsManager;
    if(!m_pFileManager) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    if(!m_pCredentialsManager) return ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    m_pCredentialsManager->GetAccessTokenCopy(m_At);

    m_entityApiRoot = GetConfigValue("api_root");
    std::string filepath = GetConfigValue("filepath");



    FileInfo* fi = RetrieveFileInfo(filepath);
    if(fi) {
        int trycount = 0;
        for(status = SendAtticPost(fi, filepath); status != ret::A_OK; trycount++) {
            status = SendAtticPost(fi, filepath);
            std::cout<<" RETRYING .................................." << std::endl;
            if(trycount > 2)
                break;
        }
    }
    else { 
        std::cout<<" INVALID FILE INFO " << std::endl;
        status = ret::A_FAIL_INVLID_FILE_INFO;
    }
    
    return status;
}

int PostFileMetadataStrategy::SendAtticPost( FileInfo* fi, const std::string& filepath) {
    std::cout<<" send attic post filepath : " << filepath << std::endl;
    int status = ret::A_OK;
    std::cout<<" SEND ATTIC POST " << std::endl;
    // Create Attic Post
    if(!fi)
        std::cout<<"invalid file info"<<std::endl;

    // Check for existing post
    std::string postid;
    fi->GetPostID(postid);

    // Construct post url
    std::string posturl;
    posturl = m_entityApiRoot;
    utils::CheckUrlAndAppendTrailingSlash(posturl);
    posturl += "posts";

    std::string relative_path;
    fi->GetFilepath(relative_path);
    std::cout<<" INSERTING RELATIVE PATH TO POST : " << relative_path << std::endl;
    bool post = true;
    Response response;
    if(postid.empty()) {
        // New Post
        std::cout<< " POST URL : " << posturl << std::endl;
        unsigned int size = utils::CheckFilesize(filepath);
        AtticPost p;
        postutils::InitializeAtticPost(fi, p, false);
        
        std::string postBuffer;
        jsn::SerializeObject(&p, postBuffer);

        std::cout<<"\n\n Attic Post Buffer : " << postBuffer << std::endl;

        status = netlib::HttpPost( posturl,
                                   NULL,
                                   postBuffer,
                                   &m_At,
                                   response );
    }
    else {
        post = false;
        // Modify Post
        posturl += "/";
        posturl += postid;

        std::cout<< " PUT URL : " << posturl << std::endl;
        
        unsigned int size = utils::CheckFilesize(filepath);
        AtticPost p;

        postutils::InitializeAtticPost(fi, p, false);
        
        std::string postBuffer;
        jsn::SerializeObject(&p, postBuffer);

        status = netlib::HttpPut( posturl,
                                  NULL,
                                  postBuffer,
                                  &m_At,
                                  response );
   }

    // Handle Response
    if(response.code == 200) {
        AtticPost p;
        jsn::DeserializeObject(&p, response.body);

        std::string postid;
        p.GetID(postid);

        if(!postid.empty()) {
            fi->SetPostID(postid); 
            if(post){
                std::string fi_filepath;
                fi->SetPostVersion(p.GetVersion()); // TODO update this in the manifest
                fi->GetFilepath(fi_filepath);

                m_pFileManager->SetFilePostId(fi_filepath, postid);
                char szVer[256] = {'\0'};
                snprintf(szVer, 256, "%d", p.GetVersion());
                m_pFileManager->SetFileVersion(fi_filepath, std::string(szVer));
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
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

FileInfo* PostFileMetadataStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = m_pFileManager->GetFileInfo(filepath);
    if(!fi)
        fi = m_pFileManager->CreateFileInfo();
    return fi;
}
