#include "metatask.h"

#include <iostream>
#include "filehandler.h"
#include "treehandler.h"
#include "getfilestrategy.h"
#include "netlib.h"
#include "posthandler.h"

namespace attic { 

MetaTask::MetaTask(FileManager* fm, 
                   CredentialsManager* cm,
                   const AccessToken& at,
                   const Entity& entity,
                   const TaskContext& context) 
                   :
                   TentTask(Task::META,
                            fm,
                            cm,
                            at,
                            entity,
                            context)
{
}

MetaTask::~MetaTask() {}


void MetaTask::RunTask() {
    std::cout<<" RUNNING META TASK " << std::endl;

    int status = ret::A_OK;
    std::string operation, filepath;
    context_.get_value("operation", operation);
    context_.get_value("filepath", filepath);

    std::cout<<" OPERATION : " << operation << std::endl;
    if(operation == "LINEAGE") {
        RetrieveFileHistory(filepath);
    }
    else if(operation == "DOWNLOAD") {
        std::string post_id, version, folderpath;
        context_.get_value("post_id", post_id);         
        context_.get_value("version", version);         
        context_.get_value("filepath", filepath);   
        DownloadFileToLocation(post_id, version, filepath);
    }
    else if(operation == "DELETE") {
        std::string post_id, version, folderpath;
        context_.get_value("post_id", post_id);
        context_.get_value("version", version);
        DeletePost(post_id, version);
    }
    else if(operation == "NEW_HEAD") {
        std::string post_id, new_version;
        context_.get_value("post_id", post_id);
        context_.get_value("version", new_version); // version to be new head
    }
    
    //Callback(status, operation);
    SetFinishedState();
}

void MetaTask::MakePostNewHead(const std::string& post_id, const std::string& version) {
    std::string msg, error_str; // callback msg and error
    // Retrieve Post at version
    std::string post_path = GetPostPath();
    std::string post_url;
    utils::FindAndReplace(post_path, "{post}", post_id, post_url);

    UrlParams params;
    params.AddValue("version", version);

    PostHandler<Post> ph(access_token());
    Post version_post;
    int status = ph.Get(post_url, &params, version_post);
    if(status == ret::A_OK) {
        status = ph.Put(post_url, NULL, version_post);
    }

    //callback
    if(callback_delegate() &&
       callback_delegate()->type() == TaskDelegate::REQUEST) {
        static_cast<RequestDelegate*>(callback_delegate())->Callback(status,
                                                                     msg.c_str(),
                                                                     error_str.c_str());
    }
}

void MetaTask::DeletePost(const std::string& post_id, const std::string& version) {
    std::string post_path = GetPostPath();
    std::string posturl;
    utils::FindAndReplace(post_path, "{post}", post_id, posturl);

    UrlParams params;
    params.AddValue("version", version);

    Response resp;
    int status = netlib::HttpDelete(posturl, &params, &access_token(), resp);
    if(status == ret::A_OK) {

    }
}

void MetaTask::DownloadFileToLocation(const std::string& post_id, 
                                      const std::string& version,
                                      const std::string& filepath) {

    // Extract folderpath
    // make sure it exists
    GetFileStrategy gfs;
    HttpStrategyContext pullcontext(file_manager(), credentials_manager());

    std::string post_path = GetPostPath();
    std::string post_attachment;
    utils::FindAndReplace(entity()->GetPreferredServer().attachment(),
                          "{entity}",
                          entity()->entity(),
                          post_attachment);

    std::string posts_feed = entity()->GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity()->entity();

    pullcontext.SetConfigValue("post_path",post_path);
    pullcontext.SetConfigValue("posts_feed", posts_feed);
    pullcontext.SetConfigValue("post_attachment", post_attachment);
    pullcontext.SetConfigValue("entity", entity);
    pullcontext.SetConfigValue("post_id", post_id);
    pullcontext.SetConfigValue("version", version);
    pullcontext.SetConfigValue("destination_folder", filepath);
    pullcontext.SetConfigValue("sync_at_post", "true");

    pullcontext.PushBack(&gfs);
    int status = pullcontext.ExecuteAll();

    std::string msg, error_str;
    // callback
    if(callback_delegate() &&
       callback_delegate()->type() == TaskDelegate::REQUEST) {
        static_cast<RequestDelegate*>(callback_delegate())->Callback(status,
                                                                     msg.c_str(),
                                                                     error_str.c_str());
    }
}

void MetaTask::RetrieveFileHistory(const std::string& filepath) {
    FileHandler fh(file_manager());
    FileInfo fi;
    if(fh.RetrieveFileInfo(filepath, fi))  {
        PostTree tree;
        int status = RetrieveFileInfoTree(fi.post_id(), tree);
        if(status == ret::A_OK) {
            std::string serialized;
            tree.ReturnSerializedTree(serialized);
            if(callback_delegate() && 
               callback_delegate()->type() == TaskDelegate::FILEHISTORY) {
                std::cout<<" FILE HISTORY CALLBACK " << std::endl;
                static_cast<HistoryDelegate*>(callback_delegate())->Callback(0,
                                                                        serialized.c_str(), 
                                                                        serialized.size(), 
                                                                        tree.node_count());
            }
        }
    }
}

int MetaTask::RetrieveFileInfoTree(const std::string& post_id, PostTree& out) {
    int status = ret::A_OK;
    std::string post_path = GetPostPath();
    TreeHandler th(access_token(), post_path);

    if(th.ConstructPostTree(post_path, post_id, out)) {
        std::cout<<" NODE COUNT : " << out.node_count() << std::endl;
    }
    else {
        std::cout<<" failed to create post tree " << std::endl;
    }

    return status;
}


} // namespace

