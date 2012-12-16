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

FileManager::FileManager() : MutexClass()
{

}

FileManager::FileManager( const std::string &manifestpath, 
                          const std::string &workingDirectory, 
                          unsigned int uFileStride)
{
    // Set manifest path
    m_ManifestFilePath = manifestpath;
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
    m_Manifest.Initialize();
    return true;
}

bool FileManager::ShutdownFileManager()
{
    m_Manifest.Shutdown();
    return true;
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
    if(split.size() < 9)
        return false;

    FileInfo* fi = CreateFileInfo ( split[0],
                                    split[1],
                                    split[2],
                                    split[3],
                                    split[4],
                                    split[5],
                                    split[6],
                                    split[7],
                                    split[8]);

    
    m_Manifest.InsertFileInfo(fi);

    return true;
}
FileInfo* FileManager::CreateFileInfo( const std::string &filename,
                                       const std::string &filepath,
                                       const std::string &chunkName,
                                       const std::string &chunkCount,
                                       const std::string &fileSize,
                                       const std::string &postId,
                                       const std::string &postVersion,
                                       unsigned char *key, // byte
                                       unsigned char *iv) // byte
{
    char* pKey = reinterpret_cast<char*>(key);
    char* pIv = reinterpret_cast<char*>(iv);

    FileInfo* fi = CreateFileInfo( filename,
                                   filepath,
                                   chunkName,
                                   chunkCount,
                                   fileSize,
                                   postId,
                                   postVersion,
                                   std::string(pKey),
                                   std::string(pIv)
                                 );
   return fi;
}


FileInfo* FileManager::CreateFileInfo( const std::string &filename,
                                       const std::string &filepath,
                                       const std::string &chunkName,
                                       const std::string &chunkCount,
                                       const std::string &fileSize,
                                       const std::string &postId,
                                       const std::string &postVersion,
                                       const std::string &key,
                                       const std::string &iv)
{
    FileInfo* fi = m_FileInfoFactory.CreateFileInfoObject();

    // Filename (str)
    fi->SetFilename(filename);
    // Filepath (str)
    fi->SetFilepath(filepath);
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
    // Key 
    fi->SetKey(key);
    // Iv
    fi->SetIv(iv);

    return fi;
}

ret::eCode FileManager::IndexFile( const std::string &filepath, 
                                   const bool insert,
                                   FileInfo* fi)
{
    // TODO :: Handle re-indexing, a file was edited, or the temporary folder was deleted
    std::cout << "Indexing file ... " << std::endl;
    ret::eCode status = ret::A_OK;
    // Create an entry
    //  Get File info
    bool reindex = true;
    if(!fi)
    {
        std::cout << " NEW FILE " << std::endl;
        fi = CreateFileInfo();
        fi->InitializeFile(filepath);
        reindex = false;
    }

    // Compress
    // Generate Compression filepath
    std::string comppath;

    GenerateCompressionPath(fi, comppath);

    status = m_Compressor.CompressFile(filepath, comppath, 1);
    if(status != ret::A_OK)
        return status;

    std::cout<< " file compressed " << std::endl;

    // Encrypt
    // Generate Crypto filepath
    std::string cryptpath;
    GenerateCryptoPath(fi, cryptpath);

    Credentials cred;
    if(!reindex)
    {
        // Generate Credentials
        cred = m_Crypto.GenerateCredentials();
        fi->SetCredentials(cred);
    }
    else
    {
        // Use existing Credentials
        cred = fi->GetCredentialsCopy();
    }

    status = m_Crypto.EncryptFile(comppath, cryptpath, cred);
    if(status != ret::A_OK)
        return status;

    std::cout<< " file encrypted " << std::endl;

    // Shove keys into a sqlite entry (and FileInfo?)

    // ChunkFile
    std::string cn;
    fi->GetChunkName(cn);
    std::cout<<" <--------------------- CHUNK NAME : " << cn << std::endl;
    // Generate Chunk Directory 
    status = m_Chunker.ChunkFile(fi, cryptpath, m_TempDirectory);
    if(status != ret::A_OK)
        return status;

    std::cout<< " file chunked " << std::endl;
    // Check if manifest is loaded
    // Write manifest entry

    if(insert)
        m_Manifest.InsertFileInfo(fi);

    return status;
}

ret::eCode FileManager::RemoveFile(const std::string &filename)
{
    ret::eCode status = ret::A_OK;

    if(!m_Manifest.RemoveFileInfo(filename))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;

    return status;
}

void FileManager::GenerateCompressionPath(FileInfo* fi, std::string &outpath)
{
    if(!fi)
        return;

    // strip any file type
    std::vector<std::string> split;
    std::string filename;
    fi->GetFilename(filename);

    utils::SplitString(filename, '.', split);
    
    if(split.size() > 0)
    { 
        //outpath = m_WorkingDirectory + "/" + split[0] + "_cmp";
        outpath = m_TempDirectory + "/" + split[0] + "_cmp";
    }
}

void FileManager::GenerateCryptoPath(FileInfo* fi, std::string &outpath)
{
    if(!fi)
        return;
    // strip any file type
    std::vector<std::string> split;
    std::string filename;
    fi->GetFilename(filename);

    utils::SplitString(filename, '.', split);
    
    if(split.size() > 0)
    { 
        //outpath = m_WorkingDirectory + "/" + split[0] + "_enc";
        outpath = m_TempDirectory + "/" + split[0] + "_enc";
    }
}

void FileManager::SetFilePostId(const std::string &filename, const std::string& postid)
{
    m_Manifest.InsertFilePostID(filename, postid);
}



ret::eCode FileManager::ConstructFile(std::string &filename)
{

    ret::eCode status = ret::A_OK;
    // Retrieve File Info from manifest

    FileInfo* fi = m_FileInfoFactory.CreateFileInfoObject();

    if(!fi)
        return ret::A_FAIL_INVALID_PTR;

    m_Manifest.QueryForFile(filename, fi);

    if(!fi)
        return ret::A_FAIL_INVALID_PTR;
    std::string chunkpath;

    // Construct outbound path
    std::string pstfx = "dchnk";
    ConstructOutboundPath( m_TempDirectory, 
                           filename, 
                           pstfx, 
                           chunkpath, 
                           true);

    std::cout << " CHUNKPATH : " << chunkpath << std::endl;

    status = m_Chunker.DeChunkFile(fi, chunkpath, m_TempDirectory);

    if(status != ret::A_OK)
        return status;

    // Decrypt chunks
    std::string decrypPath;
    pstfx.clear();
    pstfx.append("dcry");
    ConstructOutboundPath( m_TempDirectory, 
                           filename, 
                           pstfx, 
                           decrypPath, 
                           true);

    std::cout << " DECRYP PATH : " << decrypPath << std::endl;

    status = m_Crypto.DecryptFile(chunkpath, decrypPath, fi->GetCredentialsCopy());
    
    if(status != ret::A_OK)
        return status;

    // Decompress
    std::string decompPath;
    pstfx.clear();

    ConstructOutboundPath( m_TempDirectory, 
                           filename, 
                           pstfx, 
                           decompPath,
                           true);

    std::cout<< " DECOMP PATH : " << decompPath << std::endl;

    status = m_Compressor.DecompressFile(decrypPath, decompPath);

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

bool FileManager::FileExists(std::string& filepath)          
{                                                           
        std::ifstream infile(filepath.c_str());               
        bool bVal = infile.good();
        infile.close();
        return bVal;                               
}                                                           

FileInfo* FileManager::GetFileInfo(const std::string &filename)
{
    FileInfo* fi = m_FileInfoFactory.CreateFileInfoObject();

    if(!fi)
        std::cout<<"INVALID"<<std::endl;
    m_Manifest.QueryForFile(filename, fi);

    if(!fi)
        std::cout<<"INVALID"<<std::endl;
    std::cout<<"427"<<std::endl;
    if(!fi->IsValid())
        return NULL;
    return fi;
}
