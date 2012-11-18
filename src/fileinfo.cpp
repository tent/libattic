
#include "fileinfo.h"

#include <fstream>
#include <vector>

#include "utils.h"

FileInfo::FileInfo()
{

}

FileInfo::~FileInfo()
{

}

bool FileInfo::InitializeFile(std::string &szFilePath)
{
    // Check if Valid File
    //
    // Set filepath
    m_FilePath = szFilePath;
    // Extract Filename
    m_FileName = ExtractFileName(szFilePath); 
    // Check file size
    //if(!CheckFileSize())
    m_FileSize = utils::CheckFileSize(szFilePath);

    if(!m_FileSize)
        return false;

    return true;
}

std::string FileInfo::ExtractFileName(std::string &szFilePath)
{
    std::string name;
    unsigned int size = szFilePath.size();
    if(size)
    {
        // Check if passed a directory
        if(szFilePath[size-1] == '/')
            return name;

        std::vector<std::string> out;
        utils::SplitString(szFilePath, '/', out);
        if(out.size())
        {
            name = out[out.size()-1];
        }
    }

    return name;
}

FileInfoFactory::FileInfoFactory()
{

}

FileInfoFactory::~FileInfoFactory()
{
    std::deque<FileInfo*>::iterator itr;
    for(itr=m_FileList.begin(); itr != m_FileList.end(); itr++)
    {
        if(*itr)
        {
            delete *itr;
            *itr = 0;
        }
    }
}

FileInfo* FileInfoFactory::CreateFileInfoObject() 
{

    FileInfo* fp = new FileInfo();

    m_FileList.push_back(fp);

    return fp;
}
