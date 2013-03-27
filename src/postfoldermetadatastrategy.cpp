#include "postfoldermetadatastrategy.h"

#include "utils.h"
#include "netlib.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "postutils.h"
#include "folderpost.h"

PostFolderMetadataStrategy::PostFolderMetadataStrategy() {}
PostFolderMetadataStrategy::~PostFolderMetadataStrategy() {}

int PostFolderMetadataStrategy::Execute(FileManager* pFileManager,
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
    status = SendFolderPost(fi, out);

    return status;
}

int PostFolderMetadataStrategy::SendFolderPost(const FileInfo* fi, Response& out) {
    int status = ret::A_OK;

    std::string filepath, filename, parent_relative;
    fi->GetFilepath(filepath);
    fi->GetFilename(filename);
    int pos = filepath.find(filename);
    if(pos == std::string::npos) { 
        std::cout<<"MALFORMED FILEPATH " << filepath << std::endl;
        return -1;
    }
    parent_relative = filepath.substr(0, pos-1);

    Folder folder;
    std::cout<<"PARENT RELATIVE : " << parent_relative << std::endl;
    if(m_pFileManager->GetFolderInfo(parent_relative, folder)) {
        // serialize and send
        FolderPost p(folder);
        std::string postid;
        folder.GetPostID(postid);
        std::cout<<" FOLDER POST : " << postid << std::endl;

        std::string posturl;
        postutils::ConstructPostUrl(m_entityApiRoot, posturl);

        std::string postBuffer;
        jsn::SerializeObject(&p, postBuffer);

        std::cout<<" POST BUFFER : \n" << postBuffer << std::endl;

        Response response;

        bool bPost = true;
        if(postid.empty()) { // POST
            std::cout<< "FOLDER POST URL : " << posturl << std::endl;
            status = netlib::HttpPost( posturl,
                                       NULL,
                                       postBuffer,
                                       &m_At,
                                       response);

            out = response;
        }
        else { // PUT
            bPost = false;
            posturl += "/";
            posturl += postid;
            std::cout<<"FOLDER PUT URL : " << posturl << std::endl;

            status = netlib::HttpPut( posturl,
                                      NULL,
                                      postBuffer,
                                      &m_At,
                                      response );
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
                    m_pFileManager->SetFolderPostId(parent_relative, postid);
                }
            }
        }
    }
    else {
        std::cout<<" FOLDER NOT IN MANIFEST " << std::endl;
    }

    return status;
}

FileInfo* PostFolderMetadataStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = m_pFileManager->GetFileInfo(filepath);
    if(!fi)
        fi = m_pFileManager->CreateFileInfo();
    return fi;
}
