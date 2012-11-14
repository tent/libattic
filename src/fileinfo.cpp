
#include "fileinfo.h"

#include <fstream>
#include <vector>

#include "utils.h"

#include <iostream>
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
    m_filePath = szFilePath;
    // Extract Filename
    m_fileName = ExtractFileName(szFilePath); 
    std::cout<<"FILENAME : "<<m_fileName << std::endl;
    // Check file size
    if(!CheckFileSize())
        return false;

    return true;
}

bool FileInfo::CheckFileSize()
{
    std::ifstream ifs;
    ifs.open (m_filePath.c_str(), std::ifstream::binary);

    if(ifs.is_open())
    {
       ifs.seekg (0, std::ifstream::end);
       m_fileSize = ifs.tellg();
       ifs.seekg (0, std::ifstream::beg);

       ifs.close();
       return true;
    }

    return false;
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
