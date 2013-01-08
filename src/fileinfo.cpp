
#include "fileinfo.h"

#include <fstream>
#include <vector>
#include <stdio.h>

#include "chunkinfo.h"
#include "utils.h"

FileInfo::FileInfo()
{

}

FileInfo::~FileInfo()
{
    if(m_Chunks.size() > 0)
    {
        std::vector<ChunkInfo*>::iterator itr = m_Chunks.begin();

        for(;itr != m_Chunks.end(); itr++)
        {
            if(*itr)
            {

                delete *itr;
                *itr = NULL;
                std::cout<<" DELETING CHUNKINFO " << std::endl;
            }

        }

    }
}

bool FileInfo::InitializeFile(const std::string &filepath)
{
    // Check if Valid File
    //
    // Set filepath
    m_Filepath = filepath;
    // Extract Filename
    ExtractFilename(filepath, m_Filename); 
    // Check file size
    m_FileSize = utils::CheckFilesize(filepath);

    if(!m_FileSize)
        return false;

    return true;
}

void FileInfo::ExtractFilename(const std::string &filepath, std::string &out)
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


