
#include "filemanager.h"


FileManager::FileManager(std::string &szManifestFilepath)
{
    m_ManifestFilePath = szManifestFilepath;
    m_Manifest.SetFilePath(m_ManifestFilePath);
}

FileManager::~FileManager()
{

}

bool FileManager::StartupFileManager()
{
    // Load manifest
    if(!FileExists(m_ManifestFilePath))
    {
        // Create manifest
        m_Manifest.CreateEmptyManifest();
    }

    return m_Manifest.LoadManifest(m_ManifestFilePath);
}

bool FileManager::ShutdownFileManager()
{
    // Write out manifest
    return m_Manifest.WriteOutManifest();
}

bool FileManager::IndexFile(std::string &szFilePath)
{
    // Create an entry
    //  Get File info
    FileInfo* fi = CreateFileInfo();
    fi->InitializeFile(szFilePath);
    //
    // Compress
    //
    // 
    // ChunkFile
    //
    unsigned int count = m_Chunker.ChunkFile(szFilePath);
    if(!count)
        return false;

    fi->SetChunkCount(count);

    // Encrypt
    //

    // Check if manifest is loaded
    // Write manifest entry
    m_Manifest.InsertFileInfo(fi);

    return m_Manifest.WriteOutManifest();
}

FileInfo* FileManager::CreateFileInfo()
{
    return m_FileInfoFactory.CreateFileInfoObject();
}

bool FileManager::FileExists(std::string& szFilepath)          
{                                                           
        std::ifstream infile(szFilepath.c_str());               
                return infile.good();                               
}                                                           

