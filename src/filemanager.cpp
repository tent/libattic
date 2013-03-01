#include "filemanager.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

#include "filesystem.h"
#include "utils.h"
#include "chunkinfo.h"
#include "folder.h"

FileManager::FileManager() : MutexClass()
{

}

FileManager::FileManager( const std::string &manifestDirectory, 
                          const std::string &workingDirectory, 
                          unsigned int uFileStride)
{
    // Set manifest path
    m_ManifestDirectory = manifestDirectory;
    m_Manifest.SetDirectory(m_ManifestDirectory);

    // Set working directory
    m_WorkingDirectory = workingDirectory;

    // Set File Stride
    m_FileStride = uFileStride; // depricated TODO :: remove
}

FileManager::~FileManager()
{

}

bool FileManager::StartupFileManager()
{
    m_Manifest.Initialize();
    return true;
}

bool FileManager::ShutdownFileManager()
{
    m_Manifest.Shutdown();
    return true;
}

int FileManager::RemoveFile(const std::string &filepath)
{
    int status = ret::A_OK;

    Lock();
    if(!m_Manifest.RemoveFileInfo(filepath))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    Unlock();

    return status;
}

void FileManager::InsertToManifest (FileInfo* pFi) { 
    if(!pFi) return;
    // Calculate relative path
    std::string filepath, relative, canonical, parent_path, parent_relative;
    pFi->GetFilepath(filepath);
    fs::GetCanonicalPath(filepath, canonical);
    fs::MakePathRelative(m_WorkingDirectory, canonical, relative);
    fs::GetParentPath(filepath, parent_path);
    fs::MakePathRelative(m_WorkingDirectory, parent_path, parent_relative);

    std::cout<<" filepath        : " << filepath << std::endl;
    std::cout<<" canonical       : " << canonical << std::endl;
    std::cout<<" relative        : " << relative << std::endl;
    std::cout<<" working dir     : " << m_WorkingDirectory << std::endl;
    std::cout<<" parent dir      : " << parent_path << std::endl;
    std::cout<<" parent relative : " << parent_relative << std::endl;

    pFi->SetFilepath(relative);
    Lock();
    if(pFi) {
        // Insert into infotable
        m_Manifest.InsertFileInfo(*pFi);
        // Insert into folder
        Folder folder;
        if(m_Manifest.QueryForFolder(parent_relative, folder)) {
            // it exists

        }
        else {
            // does not

        }
    }

    Unlock();

}
 
void FileManager::SetFilePostId(const std::string &filepath, const std::string& postid)
{
    Lock();
    m_Manifest.InsertFilePostID(filepath, postid);
    Unlock();
}

void FileManager::SetFileChunkPostId(const std::string &filepath, const std::string& postid)
{
    m_Manifest.InsertFileChunkPostID(filepath, postid);
}

FileInfo* FileManager::CreateFileInfo()
{
    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    Unlock();

    return pFi;
}

FileInfo* FileManager::GetFileInfo(const std::string &filepath)
{
    std::string canonical, relative;
    fs::GetCanonicalPath(filepath, canonical);
    fs::MakePathRelative(m_WorkingDirectory, canonical, relative);

    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    m_Manifest.QueryForFile(relative, *pFi);
    Unlock();

    if(pFi) {
        if(!pFi->IsValid())
            return NULL;
    }

    return pFi;
}

int FileManager::GetAllFileInfo(std::vector<FileInfo>& out)
{
    Lock();
    int status = m_Manifest.QueryAllFiles(out);
    Unlock();
    return status;
}

