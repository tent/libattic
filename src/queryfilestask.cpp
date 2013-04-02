#include "queryfilestask.h"

#include <vector>
#include <string>

#include "errorcodes.h"

namespace attic { 

QueryFilesTask::QueryFilesTask( Task::TaskType type,                    
                                FileManager* pFm,                       
                                void (*callback)(int, char**, int, int))
                                :
                                ManifestTask( type,
                                              pFm,
                                              callback)
{
    m_Stride = 0;
}

QueryFilesTask::~QueryFilesTask() {}

void QueryFilesTask::RunTask() {
    std::vector<FileInfo> vec;                          
    GetFileManager()->GetAllFileInfo(vec); // blocking
    CreateCStringListsAndCallBack(vec);

    SetFinishedState();
}

int QueryFilesTask::CreateCStringListsAndCallBack(std::vector<FileInfo>& vec) {
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
