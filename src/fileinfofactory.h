
#ifndef FILEINFOFACTORY_H_
#define FILEINFOFACTORY_H_
#pragma once

#include <deque>

class FileInfo;

class FileInfoFactory
{
public:
    FileInfoFactory();
    ~FileInfoFactory();

    FileInfo* CreateFileInfoObject();

private:
    std::deque<FileInfo*> m_FileList; // List of FileInfo objects created.
};



#endif

