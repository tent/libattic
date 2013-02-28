#ifndef FOLDER_H_
#define FOLDER_H_
#pragma once

#include <string>
#include <vector>

struct FolderEntry
{
    std::string postid; 
    std::string type;   // file or folder
    std::string name;
    std::string path;
};

class Folder : public FolderEntry
{

public:
    Folder();
    ~Folder();

private:
    std::vector<FolderEntry> m_Entries;
};

#endif

