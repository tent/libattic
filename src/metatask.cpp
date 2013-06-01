#include "metatask.h"

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
    

    Callback(status, operation);
    SetFinishedState();
}


int MetaTask::RetrieveFileInfoHistory(const std::string& post_id) {
    int status = ret::A_OK;
    std::string post_path = GetPostPath();
    TreeHandler th(access_token(), post_path);

    PostTree tree;
    if(th.ConstructPostTree(post_id, tree)) {

    }
    return status;
}

} // namespace

