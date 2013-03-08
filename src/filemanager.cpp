#include "filemanager.h"

#include <string>
#include <vector>
#include <iostream>

#include "filesystem.h"
#include "utils.h"
#include "chunkinfo.h"
#include "folder.h"
#include "constants.h"

FileManager::FileManager() : MutexClass()
{

}

FileManager::FileManager( const std::string &manifestDirectory, 
                          const std::string &workingDirectory)
{
    // Set manifest path
    m_ManifestDirectory = manifestDirectory;
    m_Manifest.SetDirectory(m_ManifestDirectory);

    // Set working directory
    m_WorkingDirectory = workingDirectory;
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

    std::string test;
    GetRelativeFilepath(filepath, test);
    std::cout<<" TEST : " << test << std::endl;

    fs::GetCanonicalPath(filepath, canonical);
    fs::MakePathRelative(m_WorkingDirectory, canonical, relative);
    fs::GetParentPath(filepath, parent_path);
    fs::MakePathRelative(m_WorkingDirectory, parent_path, parent_relative);

    std::cout<<"INSERTING TO MANIFEST " << std::endl;
    std::cout<<" filepath        : " << filepath << std::endl;
    std::cout<<" canonical       : " << canonical << std::endl;
    std::cout<<" relative        : " << relative << std::endl;
    std::cout<<" working dir     : " << m_WorkingDirectory << std::endl;
    std::cout<<" parent dir      : " << parent_path << std::endl;
    std::cout<<" parent relative : " << parent_relative << std::endl;

    //if(parent_relative.empty())
    if(canonical.empty()) {
        parent_relative = cnst::g_szWorkingPlaceHolder;
        relative = test;
    }

    std::cout<<" PARENT_RELATIVE NOW : " << parent_relative << std::endl;
    std::cout<<" RELATIVE : " << relative << std::endl;

    pFi->SetFilepath(relative);
    Lock();
    // Insert into infotable
    m_Manifest.InsertFileInfo(*pFi);
    Unlock();

    // Folder operations
    Lock();
    if(!m_Manifest.IsFolderInManifest(parent_relative)){
        // Create folder entry
        m_Manifest.InsertFolderInfo(parent_relative, "");
    }
    Unlock();

    // File operations
    Lock();
    std::string folderid;
    if(!m_Manifest.GetFolderID(parent_relative, folderid)) {
        std::cout<<" failed to find folder post " << std::endl;
    }

    std::string postid;
    pFi->GetPostID(postid);
    if(!m_Manifest.IsFolderEntryInManifest(relative)) {
       m_Manifest.InsertFolderEnrty(folderid,   
                                    postid, 
                                    cnst::g_szFileType,       
                                    relative);  
    }
    Unlock();
}
 
void FileManager::SetFilePostId(const std::string &filepath, const std::string& postid)
{
    std::string path, relative, canonical, parent_path, parent_relative;

    path += m_WorkingDirectory;
    utils::CheckUrlAndAppendTrailingSlash(path);
    path += filepath;

    fs::GetCanonicalPath(path, canonical);
    fs::MakePathRelative(m_WorkingDirectory, canonical, relative);
    fs::GetParentPath(canonical, parent_path);
    fs::MakePathRelative(m_WorkingDirectory, parent_path, parent_relative);


    std::cout<<" ** setpostid filepath : " << filepath << std::endl;
    std::cout<<" ** canonical : " << canonical << std::endl;
    std::cout<<" ** relative : " << relative << std::endl;
    std::cout<<" ** parent : " << parent_path << std::endl;
    std::cout<<" ** parent relative : " << parent_relative << std::endl;

    Lock();
    m_Manifest.InsertFilePostID(relative, postid);
    Unlock();
    //m_Manifest.InsertFilePostID(filepath, postid);

    if(parent_relative.empty())
        parent_relative = cnst::g_szWorkingPlaceHolder;

    // Update folder entry
    Lock();
    if(m_Manifest.IsFolderEntryInManifest(relative))
        m_Manifest.SetFolderEntryMetapostID(relative, postid);
    else 
        std::cout<< "Could not find folder entry in MANIFEST " << std::endl;
    Unlock();
}

void FileManager::SetFolderPostId(const std::string& folderpath, const std::string& postid)
{
    std::string parent_relative = folderpath;

    if(parent_relative.empty())
        parent_relative = cnst::g_szWorkingPlaceHolder;

    std::cout<<" FOLDER PATH : " << folderpath << std::endl;
    std::cout<<" SETTING FOLDER POST ID : " << parent_relative << std::endl;

    Lock();
    if(m_Manifest.IsFolderInManifest(parent_relative))
        m_Manifest.SetFolderPostID(parent_relative, postid);
    else
        std::cout<<" COULD NOT FIND FOLDER IN MANIFEST " << std::endl;
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

void FileManager::GetRelativeFilepath(const std::string& filepath, std::string& out)
{
    std::string canonical, relative;
    fs::GetCanonicalPath(filepath, canonical);
    fs::MakePathRelative(m_WorkingDirectory, canonical, relative);

    std::cout<<" GET FILE INFO : " << std::endl;
    std::cout<<" \t filepath : " << filepath << std::endl;
    std::cout<<" \t canonical : " << canonical << std::endl;
    std::cout<<" \t relatvie : " << relative << std::endl;
    std::cout<<" \t working : " << m_WorkingDirectory << std::endl;
    
    if(canonical.empty()) {
        relative = filepath;
        if(relative.find("/") == std::string::npos){// && relative.find("\\") == std::string::npos) {
            canonical = m_WorkingDirectory + "/" + relative;

            std::cout<<" new canonical : " << canonical << std::endl;
            fs::MakePathRelative(m_WorkingDirectory, canonical, relative);
        }
        else {
            std::cout<<" not npos : " <<relative.find("/") << std::endl;
        }


        std::cout<<" NEW RELATIVE : " << relative << std::endl;
    }
    out = relative;
}

bool FileManager::DoesFileExist(const std::string& filepath)
{
    std::string relative;
    GetRelativeFilepath(filepath, relative);
    Lock();
    bool stat = m_Manifest.IsFileInManifest(relative);
    Unlock();

    return stat;
}

FileInfo* FileManager::GetFileInfo(const std::string &filepath)
{
    std::string canonical, relative;
    fs::GetCanonicalPath(filepath, canonical);
    fs::MakePathRelative(m_WorkingDirectory, canonical, relative);

    std::cout<<" GET FILE INFO : " << std::endl;
    std::cout<<" \t filepath : " << filepath << std::endl;
    std::cout<<" \t canonical : " << canonical << std::endl;
    std::cout<<" \t relatvie : " << relative << std::endl;
    
    if(canonical.empty()) {
        relative = filepath;
        if(relative.find("/") == std::string::npos && relative.find("\\") == std::string::npos) {
            canonical = m_WorkingDirectory + "/" + relative;
            fs::MakePathRelative(m_WorkingDirectory, canonical, relative);
        }

        std::cout<<" NEW RELATIVE : " << relative << std::endl;
    }

    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    m_Manifest.QueryForFile(relative, *pFi);
   // m_Manifest.QueryForFile(filepath, *pFi);
    Unlock();

    if(pFi) {
        if(!pFi->IsValid())
            return NULL;
    }

    return pFi;
}

bool FileManager::GetFolderInfo(const std::string& folderpath, Folder& folder)
{
    std::cout<<" GET FOLDER INFO : " << folderpath << std::endl;
    std::string parent_relative = folderpath;

    if(parent_relative.empty())
        parent_relative = cnst::g_szWorkingPlaceHolder;

    bool bRet = false;
    Lock();
    if(m_Manifest.IsFolderInManifest(parent_relative)) {
        std::string folderpostid, folder_id; 
        m_Manifest.GetFolderPostID(parent_relative, folderpostid);
        m_Manifest.GetFolderID(parent_relative, folder_id);
        folder.SetPostID(folderpostid);
        folder.SetPath(parent_relative);
        // Retreive Entries
        std::vector<FolderEntry> entries;
        m_Manifest.GetFolderEntries(folder_id, entries);

        std::cout<<" NUMBER OF ENTRIES : " << entries.size() << std::endl;
        for(unsigned int i=0; i<entries.size(); i++)
            folder.PushBackEntry(entries[i]);
        
        bRet = true;
    }
    Unlock();

    return bRet;
}

int FileManager::GetAllFileInfo(std::vector<FileInfo>& out)
{
    Lock();
    int status = m_Manifest.QueryAllFiles(out);
    Unlock();
    return status;
}

