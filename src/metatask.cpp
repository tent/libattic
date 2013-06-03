#include "metatask.h"

#include <iostream>
#include "filehandler.h"
#include "treehandler.h"

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

    if(operation == "LINEAGE") {
        FileHandler fh(file_manager());
        FileInfo fi;
        if(fh.RetrieveFileInfo(filepath, fi)) 
            status = RetrieveFileInfoHistory(fi.post_id());
    }
    
    std::cout<<"calling back? " << std::endl;
    if(callback_delegate() && callback_delegate()->type() == TaskDelegate::FILEHISTORY) {
        std::cout<<" FILE HISTORY CALLBACK " << std::endl;
        std::string str("test callback");
        static_cast<HistoryCallback*>(callback_delegate())->Callback(1, str.c_str(), 2, 3);
    }
    else
        Callback(status, operation);
    SetFinishedState();
}

int MetaTask::RetrieveFileInfoHistory(const std::string& post_id) {
    int status = ret::A_OK;
    std::string post_path = GetPostPath();
    TreeHandler th(access_token(), post_path);

    PostTree tree;
    if(th.ConstructPostTree(post_id, tree)) {
        std::cout<<" NODE COUNT : " << tree.node_count() << std::endl;
    }
    else {
        std::cout<<" failed to create post tree " << std::endl;
    }

    return status;
}


} // namespace

