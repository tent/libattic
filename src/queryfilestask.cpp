#include "queryfilestask.h"

#include <vector>
#include <string>

#include "errorcodes.h"


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

QueryFilesTask::~QueryFilesTask()
{

}

void QueryFilesTask::RunTask()
{


    std::vector<FileInfo> vec;                          
    GetFileManager()->GetAllFileInfo(vec); // blocking
    CreateCStringListsAndCallBack(vec);

    SetFinishedState();
}

int QueryFilesTask::CreateCStringListsAndCallBack(std::vector<FileInfo>& vec)
{
    int status = ret::A_OK;

    unsigned int totalcount = vec.size();

    if(m_Stride == 0) m_Stride = 1;

    int size = 0;
    for(unsigned int i=0; i<totalcount; i+=m_Stride)
    {
        if((i+m_Stride) < totalcount)
            size = m_Stride;
        else
            size = (totalcount - i);
        
        char** buf = new char*[size];

        std::string fp;
        for(unsigned int j=0; j<size; j++)
        {
           fp.clear(); 
           vec[i+j].GetFilepath(fp); 
           buf[j] = new char[fp.size()+1];
           memset(buf[j], '\0', fp.size()+1);
           memcpy(buf[j], fp.c_str(), fp.size());
        }
        Callback(status, buf, size, totalcount);
    }

}


