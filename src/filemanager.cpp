#include "filemanager.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

#include "utils.h"

// TODO :: considering removing all file reading/writing out of 
//         manifest to filemanager. (centralize all file writing)
//

FileManager::FileManager()
{

}

FileManager::FileManager( const std::string &szManifestFilepath, 
                          const std::string &workingDirectory, 
                          unsigned int uFileStride)
{
    // Set manifest path
    m_ManifestFilePath = szManifestFilepath;
    m_Manifest.SetFilePath(m_ManifestFilePath);

    // Set working directory
    m_WorkingDirectory = workingDirectory;

    // Set File Stride
    m_FileStride = uFileStride;
    m_Chunker.SetChunkSize(m_FileStride);
    m_Crypto.SetStride(m_FileStride);
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

    return LoadManifest(m_ManifestFilePath);
}

bool FileManager::ShutdownFileManager()
{
    // Write out manifest
    return m_Manifest.WriteOutManifest();
}

bool FileManager::LoadManifest(const std::string &szFilePath)
{
    m_ifStream.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(m_ifStream.is_open())
    {
        unsigned int line_count = 0;
        std::string line;
        bool ok;
        // Read Line by line
        while(!m_ifStream.eof())
        {
            std::getline(m_ifStream, line);
            if(line_count == 0)
            {
                // header info
                std::cout<<"HEADER INFO : "<<line<<std::endl;
                ok = ReadInHeader(line);
            }
            else
            {
                // Its an entry
                std::cout<<"ENTRY INFO : "<<line<<std::endl;
                ok = ReadInEntry(line);
            }
            
            if(!ok)
            {
                std::cout<< " FAILED TO READ IN HEADER " << std::endl;
                //m_ifStream.close();
                //return false;
            }
            if(m_Manifest.GetEntryCount() == line_count)
                break;

            line_count++;
        }

        m_ifStream.close();
        return true;
    }

    return false;
}

bool FileManager::WriteOutChanges()
{
    return m_Manifest.WriteOutManifest(); 
}

bool FileManager::ReadInHeader(std::string &h)
{ 
    // Split string
    std::vector<std::string> split;
    utils::SplitString(h, '\t', split);

    // Check for correct number of params
    if(split.size() < 1)
        return false;


    // Entry Count
    m_Manifest.SetEntryCount(atoi(split[0].c_str()));
    return true;
}

bool FileManager::ReadInEntry(std::string &e)
{
    // Split string
    std::vector<std::string> split;
    utils::SplitString(e, '\t', split);
    
    // Check for correct number of params
    if(split.size() < 7)
        return false;

    FileInfo* fi = CreateFileInfo ( split[0],
                                    split[1],
                                    split[2],
                                    split[3],
                                    split[4],
                                    split[5],
                                    split[6] );
    
    m_Manifest.InsertFileInfo(fi);

    return true;
}
FileInfo* FileManager::CreateFileInfo( const std::string &filename,
                                       const std::string &filepath,
                                       const std::string &chunkName,
                                       const std::string &chunkCount,
                                       const std::string &fileSize,
                                       const std::string &postId,
                                       const std::string &postVersion)
{
    FileInfo* fi = m_FileInfoFactory.CreateFileInfoObject();

    // Filename (str)
    fi->SetFileName(filename);
    // Filepath (str)
    fi->SetFilePath(filepath);
    // ChunkName (str)
    fi->SetChunkName(chunkName);
    // ChunkCount (unsigned int)
    fi->SetChunkCount((unsigned)atoi(chunkCount.c_str()));
    // FileSize (unsigned int)
    fi->SetFileSize((unsigned)atoi(fileSize.c_str()));
    // Post ID
    fi->SetPostID(postId);
    // Post Version
    fi->SetPostVersion((unsigned)atoi(postVersion.c_str()));

    return fi;
}



ret::eCode FileManager::IndexFile(const std::string &szFilePath)
{
    ret::eCode status = ret::A_OK;
    // Create an entry
    //  Get File info
    FileInfo* fi = CreateFileInfo();
    fi->InitializeFile(szFilePath);

    //
    // Compress
    // Generate Compression filepath
    std::string comppath;

    GenerateCompressionPath(fi, comppath);

    status = m_Compressor.CompressFile(szFilePath, comppath, 1);
    if(status != ret::A_OK)
        return status;

    // Encrypt
    // Generate Crypto filepath
    std::string cryptpath;
    GenerateCryptoPath(fi, cryptpath);
    // Generate Credentials
    Credentials cred = m_Crypto.GenerateCredentials();
    fi->SetCredentials(cred);

    status = m_Crypto.EncryptFile(comppath, cryptpath, cred);
    if(status != ret::A_OK)
        return status;

    // Shove keys into a sqlite entry (and FileInfo?)

    // ChunkFile
    // Generate Chunk Directory 
    status = m_Chunker.ChunkFile(fi, cryptpath, m_TempDirectory);
    if(status != ret::A_OK)
        return status;

    // Check if manifest is loaded
    // Write manifest entry
    m_Manifest.InsertFileInfo(fi);

    bool success = m_Manifest.WriteOutManifest();
    return status;
}

ret::eCode FileManager::RemoveFile(const std::string &szFileName)
{
    ret::eCode status = ret::A_OK;

    if(!m_Manifest.RemoveFileInfo(szFileName))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;

    return status;
}

void FileManager::GenerateCompressionPath(FileInfo* fi, std::string &szOutPath)
{
    if(!fi)
        return;

    // strip any file type
    std::vector<std::string> split;
    utils::SplitString(fi->GetFileName(), '.', split);
    
    if(split.size() > 0)
    { 
        //szOutPath = m_WorkingDirectory + "/" + split[0] + "_cmp";
        szOutPath = m_TempDirectory + "/" + split[0] + "_cmp";
    }
}

void FileManager::GenerateCryptoPath(FileInfo* fi, std::string &szOutPath)
{
    if(!fi)
        return;
    // strip any file type
    std::vector<std::string> split;
    utils::SplitString(fi->GetFileName(), '.', split);
    
    if(split.size() > 0)
    { 
        //szOutPath = m_WorkingDirectory + "/" + split[0] + "_enc";
        szOutPath = m_TempDirectory + "/" + split[0] + "_enc";
    }
}

ret::eCode FileManager::ConstructFile(std::string &filename)
{

    ret::eCode status = ret::A_OK;
    // Retrieve File Info from manifest
    FileInfo *fi = m_Manifest.RetrieveFileInfo(filename);

    if(!fi)
        return ret::A_FAIL_INVALID_PTR;

    /*
    // Construct outbound path
    std::string pstfx = "dchnk";
    std::string chunkPath = ConstructOutboundPath(m_WorkingDirectory, true, filename, pstfx);
    // De-chunk
    status = m_Chunker.DeChunkFile(fi, chunkPath, m_WorkingDirectory);

    if(status != ret::A_OK)
        return status;
    */

    /*
    // Decrypt chunks
    pstfx.clear();
    pstfx.append("dcry");
    std::string decrypPath = ConstructOutboundPath(m_WorkingDirectory, true, filename, pstfx);
    status = m_Crypto.DecryptFile(chunkPath, decrypPath, fi->GetCredentials());
    
    if(status != ret::A_OK)
        return status;
    */

    // Decompress
    //pstfx.clear();

    /*
    std::string pstfx; // temporary remove when adding back encryption and chunking
    std::string decompPath = ConstructOutboundPath(m_WorkingDirectory, false, filename, pstfx);
    status = m_Compressor.DecompressFile(decrypPath, decompPath);

    */
    //if(status != ret::A_OK) // Currently useless check, here for consistency's sake.
    //    return status;

    return status;
}

void FileManager::ConstructOutboundPath( const std::string &workingDir, 
                                         const std::string &filename, 
                                         const std::string &postfix,
                                         std::string &outboundPath,
                                         bool bStripFileType)
{
    std::string path = workingDir;

    std::string fn = filename;

    if(bStripFileType)
    {
        std::vector<std::string> out;
        utils::SplitString(fn, '.', out);

        if(out.size() > 0)
        {
            fn = out[0];
        }
    }

    // Check for trailing /

    if(path[path.size()-1] != '/')
        path.append("/");

    // Attach postfix to filename
    fn += postfix;

    path += fn;
    
    outboundPath = path;
}

FileInfo* FileManager::CreateFileInfo()
{
    return m_FileInfoFactory.CreateFileInfoObject();
}

bool FileManager::FindFileInManifest(const std::string &filename)
{
    return m_Manifest.IsFileInManifest(filename);
}

bool FileManager::FileExists(std::string& szFilepath)          
{                                                           
        std::ifstream infile(szFilepath.c_str());               
        bool bVal = infile.good();
        infile.close();
        return bVal;                               
}                                                           

FileInfo* FileManager::GetFileInfo(const std::string &filename)
{
    return m_Manifest.RetrieveFileInfo(filename);
}
