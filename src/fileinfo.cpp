
#include "fileinfo.h"

#include <fstream>

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
    //
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

