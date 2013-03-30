#include "fileinfofactory.h"

#include "fileinfo.h"

namespace attic { 

FileInfoFactory::FileInfoFactory()
{

}

FileInfoFactory::~FileInfoFactory()
{
    std::deque<FileInfo*>::iterator itr = m_FileList.begin();
    for(;itr != m_FileList.end();)
    {
        if(*itr)
        {
            delete *itr;
            *itr = NULL;
        }
        itr++;
        m_FileList.pop_front();
    }

    m_FileList.clear();
}

FileInfo* FileInfoFactory::CreateFileInfoObject() 
{
    FileInfo* fp = new FileInfo();

    m_FileList.push_back(fp);

    return fp;
}

}//namespace
