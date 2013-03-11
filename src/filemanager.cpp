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

void FileManager::ExtractRelativePaths(const FileInfo* pFi, 
                                       std::string& relative_out, 
                                       std::string& parent_relative_out)
{
    std::string filepath, relative, canonical;
    pFi->GetFilepath(filepath);

    // If filepath is already relative
    if(IsPathRelative(filepath)) {
        // set canonical
        std::cout<<" PATH IS RELATIVE " << std::endl;
        int pos = filepath.find(cnst::g_szWorkingPlaceHolder);
        canonical = m_WorkingDirectory + filepath.substr((pos+strlen(cnst::g_szWorkingPlaceHolder)));
        std::cout<<" working : " << m_WorkingDirectory << std::endl;
        std::cout<<" CANONICAL IS : " << canonical << std::endl;
        relative = filepath;
    }
    else {
        fs::GetCanonicalPath(filepath, canonical);
        fs::MakePathRelative(m_WorkingDirectory, canonical, relative);
        // TODO :: fix for windows
        relative = std::string(cnst::g_szWorkingPlaceHolder) + "/" + relative;

        std::cout<<" RELATIVE : " << relative << std::endl;
        //if(parent_relative.empty())
        if(canonical.empty()) {
            std::cout<<" FILEMANAGER - CANONICAL IS EMPTY - " << filepath << std::endl;
        }
    }
    // Extract parent relative :
    std::string filename;
    pFi->GetFilename(filename);
    int pos = relative.find(filename);
    std::string parent_relative;
    if(pos != std::string::npos)
        parent_relative = relative.substr(0, pos-1); // minus 1 to include /

    std::cout<<"INSERTING TO MANIFEST " << std::endl;
    std::cout<<" filepath        : " << filepath << std::endl;
    std::cout<<" canonical       : " << canonical << std::endl;
    std::cout<<" relative        : " << relative << std::endl;
    std::cout<<" parent_relative : " << parent_relative << std::endl;

    relative_out = relative;
    parent_relative_out = parent_relative;
}

void FileManager::InsertToManifest (FileInfo* pFi) { 
    if(!pFi) return;
    // Calculate relative path
    std::string relative, parent;
    ExtractRelativePaths(pFi, relative, parent);

    pFi->SetFilepath(relative);
    Lock();
    // Insert into infotable
    m_Manifest.InsertFileInfo(*pFi);
    Unlock();

    // Folder operations
    Lock();
    if(!m_Manifest.IsFolderInManifest(parent)){
        // Create folder entry
        m_Manifest.InsertFolderInfo(parent, "");
    }
    Unlock();

    // File operations
    Lock();
    std::string folderid;
    if(!m_Manifest.GetFolderID(parent, folderid)) {
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
 

void FileManager::SetFileVersion(const std::string& filepath, const std::string& version)
{
    if(IsPathRelative(filepath)) {
        Lock();
        m_Manifest.UpdateFileVersion(filepath, version);
        Unlock();
    }
}

void FileManager::SetFilePostId(const std::string &filepath, const std::string& postid)
{
    if(IsPathRelative(filepath)) {
        Lock();
        m_Manifest.UpdateFilePostID(filepath, postid);
        Unlock();

        Lock();
        if(m_Manifest.IsFolderEntryInManifest(filepath))
            m_Manifest.SetFolderEntryMetapostID(filepath, postid);
        else 
            std::cout<< "Could not find folder entry in MANIFEST " << std::endl;
        Unlock();
    }
    else {
        std::cout<<" FILEPATH NOT RELATIVE IN SET POST ID : " << filepath << std::endl;
    }
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
        m_Manifest.UpdateFolderPostID(parent_relative, postid);
    else
        std::cout<<" COULD NOT FIND FOLDER IN MANIFEST " << std::endl;
    Unlock();
 
}

void FileManager::SetFileChunkPostId(const std::string &filepath, const std::string& postid)
{
    m_Manifest.UpdateFileChunkPostID(filepath, postid);
}

FileInfo* FileManager::CreateFileInfo() {
    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    Unlock();

    return pFi;
}

void FileManager::GetCanonicalFilepath(const std::string& relativepath, std::string& out) {
    std::string relative, canonical;

    std::cout<<" RELATIVE IN : " << relativepath << std::endl;
    if(IsPathRelative(relativepath)) {
        int pos = relativepath.find(cnst::g_szWorkingPlaceHolder);
        if(pos != std::string::npos) {
            out = m_WorkingDirectory + "/" + relativepath.substr(pos + strlen(cnst::g_szWorkingPlaceHolder) + 1);
            std::cout<<" OUT : " << out << std::endl;
        }
        else {
            std::cout << " MALFORMED RELATIVE PATH " << relativepath << std::endl;
        }
    }
    else {
        std::cout<< " PATH NOT RELATIVE " << std::endl;
    }

}

bool FileManager::IsPathRelative(const std::string& filepath)
{
    if(filepath.find(cnst::g_szWorkingPlaceHolder) != std::string::npos)
        return true;
    return false;
}

bool FileManager::DoesFileExist(const std::string& filepath)
{
    bool stat = false;
    if(IsPathRelative(filepath)) {
        Lock();
        stat = m_Manifest.IsFileInManifest(filepath);
        Unlock();
    }
    else {
        std::cout<<" FILEPATH PASSED NOT RELATIVE : "<< filepath << std::endl;
    }

    return stat;
}

bool FileManager::AttemptToGetRelativePath(const std::string& filepath, std::string& out)
{
    bool retval = false;
    std::cout<<" attempting to get a relative path " << std::endl;
    std::cout<<" for : " << filepath << std::endl;
    std::string rel;
    fs::MakePathRelative(m_WorkingDirectory, filepath, rel);
    std::cout<<" rel : " << rel << std::endl;
    std::string canonical;
    int pos = 0;
    int last = 0;
    while(pos != std::string::npos) {
        last = pos;
        pos = filepath.find("/", pos + 1);
    }

    if(last) {
        fs::GetCanonicalPath(filepath.substr(0, pos-1), canonical);
        std::cout<<"can?onical? : " << canonical << std::endl;
        canonical += filepath.substr(pos+1);
    }
    else {
        std::cout<<" getting canonical " << std::endl;
        fs::GetCanonicalPath(filepath, canonical);
    }
    std::cout<<" canonical : " << canonical << std::endl;

    if(canonical.find(m_WorkingDirectory) != std::string::npos) {
        std::string relative;
        fs::MakePathRelative(m_WorkingDirectory, canonical, relative);
        out = std::string(cnst::g_szWorkingPlaceHolder) + "/" + relative;
        retval = true;
    }
    else {
        std::cout<< " could not find working directory within canonical path " << std::endl;
    }
    return retval;
} 

FileInfo* FileManager::GetFileInfo(const std::string &filepath)
{
    FileInfo* pFi = NULL;
    std::string relative;
    if(!IsPathRelative(filepath)) {
        AttemptToGetRelativePath(filepath, relative);
    } 
    else
        relative = filepath;

    // Attempt to get relative path
    if(IsPathRelative(relative)) {
        Lock();
        pFi = m_FileInfoFactory.CreateFileInfoObject();
        m_Manifest.QueryForFile(relative, *pFi);
        Unlock();

        if(pFi) {
            if(!pFi->IsValid()) {
                pFi = NULL;
            }
        }
        else {
            std::cout<<" NULL FileInfo " << std::endl;
        }
    }
    else {
        std::cout<<"GETFILEINFO FILEPATH PASSED NOT RELATIVE : "<< filepath << std::endl;
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

