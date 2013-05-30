#include "queryfilestask.h"


#include <string>

#include "errorcodes.h"

namespace attic { 

QueryFilesTask::QueryFilesTask(FileManager* pFm,                       
                               const TaskContext& context)
                               :
                               ManifestTask(Task::QUERYMANIFEST,
                                            pFm,
                                            context) {
    m_Stride = 0;
}

QueryFilesTask::~QueryFilesTask() {}

void QueryFilesTask::RunTask() {
    std::deque<FileInfo> vec;                          
    file_manager()->GetAllFileInfo(vec); // blocking
    CreateCStringListsAndCallBack(vec);

    SetFinishedState();
}

int QueryFilesTask::CreateCStringListsAndCallBack(std::deque<FileInfo>& vec) {
    int status = ret::A_OK;
    unsigned int size = vec.size();
    char** buf = new char*[size];
    for(int j=0; j<size; j++) {
        std::string fp = vec[j].filepath();

        std::cout<< " pushing back : " << fp << std::endl; 
        buf[j] = new char[fp.size()+1];
        memset(buf[j], '\0', fp.size()+1);
        memcpy(buf[j], fp.c_str(), fp.size());
    }
    Callback(status, buf, size, size);
}

} //namespace
