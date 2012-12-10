
#include "fileinfo.h"

#include <fstream>
#include <vector>

#include <stdio.h>

#include "utils.h"


bool FileInfo::InitializeFile(const std::string &filepath)
{
    // Check if Valid File
    //
    // Set filepath
    m_FilePath = filepath;
    // Extract Filename
    ExtractFileName(filepath, m_FileName); 
    // Check file size
    m_FileSize = utils::CheckFileSize(filepath);

    if(!m_FileSize)
        return false;

    return true;
}

void FileInfo::ExtractFileName(const std::string &filepath, std::string &out)
{
    std::string name;
    unsigned int size = filepath.size();
    if(size)
    {
        // Check if passed a directory
        if(filepath[size-1] != '/')
        {
            std::vector<std::string> out;
            utils::SplitString(filepath, '/', out);
            if(out.size())
            {
                name = out[out.size()-1];
            }
        }
    }
    out = name;
}

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
