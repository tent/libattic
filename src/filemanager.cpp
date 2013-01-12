#include "filemanager.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

#include "utils.h"
#include "chunkinfo.h"

// TODO :: considering removing all file reading/writing out of 
//         manifest to filemanager. (centralize all file writing)
//

static const char* szCompSuffix = "_cmp";
static const char* szEncryptSuffix = "_enc";

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

    FileInfo* pFi = CreateFileInfo ( split[0],
                                    split[1],
                                    split[2],
                                    split[3],
                                    split[4],
                                    split[5],
                                    split[6],
                                    split[7],
                                    split[8]);

    
    m_Manifest.InsertFileInfo(pFi);

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

    FileInfo* pFi = CreateFileInfo( filename,
                                   filepath,
                                   chunkName,
                                   chunkCount,
                                   fileSize,
                                   postId,
                                   postVersion,
                                   std::string(pKey),
                                   std::string(pIv)
                                 );
   return pFi;
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
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();

    // Filename (str)
    pFi->SetFilename(filename);
    // Filepath (str)
    pFi->SetFilepath(filepath);
    // ChunkName (str)
    pFi->SetChunkName(chunkName);
    // ChunkCount (unsigned int)
    pFi->SetChunkCount((unsigned)atoi(chunkCount.c_str()));
    // FileSize (unsigned int)
    pFi->SetFileSize((unsigned)atoi(fileSize.c_str()));
    // Post ID
    pFi->SetPostID(postId);
    // Post Version
    pFi->SetPostVersion((unsigned)atoi(postVersion.c_str()));
    // Key 
    pFi->SetKey(key);
    // Iv
    pFi->SetIv(iv);

    return pFi;
}

void FileManager::GenerateCompressionPath(std::string filename, std::string &outpath)
{
    // strip any file type
    std::vector<std::string> split;
    utils::SplitString(filename, '.', split);
    
    if(split.size() > 0)
    { 
        outpath = m_TempDirectory + "/" + split[0] + szCompSuffix;
    }

}

void FileManager::GenerateEncryptionPath(std::string filename, std::string &outpath)
{
    std::vector<std::string> split;
    utils::SplitString(filename, '.', split);
    
    if(split.size() > 0)
    { 
        outpath = m_TempDirectory + "/" + split[0] + szEncryptSuffix;
    }
}

int FileManager::ChunkFile(FileInfo* pFi)
{
    int status = ret::A_OK;

    if(pFi)
    {
        std::string filepath;
        pFi->GetFilepath(filepath);

        // Generate Chunk Directory 
        status = m_Chunker.ChunkFile(pFi, filepath, m_TempDirectory);
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }
        
    return status;
}

int FileManager::CompressChunks(FileInfo* pFi)
{
    int status = ret::A_OK;

    if(pFi)
    {
        std::vector<ChunkInfo*>* pInfo = pFi->GetChunkInfoList();
        std::vector<ChunkInfo*>::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {

            if(*itr)
            {
                std::string chunkname;
                (*itr)->GetChunkName(chunkname);
                
                std::string comppath;
                GenerateCompressionPath(chunkname, comppath);

                std::string filepath;
                filepath += m_TempDirectory + "/" + chunkname;

                status = m_Compressor.CompressFile(filepath, comppath, 1);

                if(status != ret::A_OK)
                    break;
            }

            std::cout<< " file compressed " << std::endl;
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::EncryptCompressedChunks(FileInfo* pFi)
{
    int status = ret::A_OK;

    if(pFi)
    {
        // Generate Credentials
        Credentials cred;
        m_MasterKey.GetMasterKeyCredentials(cred);
        
        std::vector<ChunkInfo*>* pInfo = pFi->GetChunkInfoList();
        std::vector<ChunkInfo*>::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {
            if(*itr)
            {
                std::string chunkname;
                (*itr)->GetChunkName(chunkname);
                
                // Generate unique Iv for chunk
                std::string iv;
                m_Crypto.GenerateIv(iv);
                (*itr)->SetIv(iv);
                status = cred.SetIv(iv);

                std::string hash;
                m_Crypto.GenerateHash(iv, hash);
                (*itr)->SetCipherTextSum(hash);

                if(status != ret::A_OK)
                    break;

                std::string comppath;
                GenerateCompressionPath(chunkname, comppath);

                std::string encpath;
                GenerateEncryptionPath(chunkname, encpath);

                status = m_Crypto.EncryptFile(comppath, encpath, cred);

                if(status != ret::A_OK)
                    break;
            }
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::IndexFileNew( const std::string& filepath,
                               const bool insert,
                               FileInfo* pFi)
{
    // Chunk
    // Compress
    // Encrpyt
    // Post
    
    // TODO :: Handle re-indexing, a file was edited, or the temporary folder was deleted
    std::cout << "Indexing file ... " << std::endl;
    int status = ret::A_OK;

    if(!m_MasterKey.IsEmpty())
        status = ret::A_FAIL_INVALID_MASTERKEY;
      std::cout<< " file chunked " << std::endl;
   
    if(status == ret::A_OK)
    {
        // Create an entry
        //  Get File info
        bool reindex = true;
        if(!pFi)
        {
            std::cout << " NEW FILE " << std::endl;
            pFi = CreateFileInfo();
            pFi->InitializeFile(filepath);
            reindex = false;
        }
 
        // Chunk File
        status = ChunkFile(pFi);
        if(status != ret::A_OK)
            return status;

        // Compress Chunks
        status = CompressChunks(pFi);
        if(status != ret::A_OK)
            return status;

        // Encrypt Chunks
        status = EncryptCompressedChunks(pFi);
        if(status != ret::A_OK)
            return status;


        // Shove keys into a sqlite entry (and FileInfo?)

        // Check if manifest is loaded
        // Write manifest entry
        if(insert)
            m_Manifest.InsertFileInfo(pFi);


        std::string jsontest;
        pFi->GetSerializedChunkData(jsontest);
        std::cout<<" SERIALIZED DATA : \n"<< jsontest << std::endl;
        pFi->LoadSerializedChunkData(jsontest);
    }

    return status;
}


ret::eCode FileManager::IndexFile( const std::string &filepath, 
                                   const bool insert,
                                   FileInfo* pFi)
{
    // TODO :: Handle re-indexing, a file was edited, or the temporary folder was deleted
    std::cout << "Indexing file ... " << std::endl;

    if(!m_MasterKey.IsEmpty())
        return ret::A_FAIL_INVALID_MASTERKEY;

    ret::eCode status = ret::A_OK;
    // Create an entry
    //  Get File info
    bool reindex = true;
    if(!pFi)
    {
        std::cout << " NEW FILE " << std::endl;
        pFi = CreateFileInfo();
        pFi->InitializeFile(filepath);
        reindex = false;
    }

    // Compress
    // Generate Compression filepath
    std::string comppath;

    GenerateCompressionPath(pFi, comppath);

    status = m_Compressor.CompressFile(filepath, comppath, 1);
    if(status != ret::A_OK)
        return status;

    std::cout<< " file compressed " << std::endl;

    // Encrypt
    // Generate Crypto filepath
    std::string cryptpath;
    GenerateCryptoPath(pFi, cryptpath);

    Credentials cred;
    if(!reindex)
    {
        m_MasterKey.GetMasterKeyCredentials(cred);

        // Set random iv
        std::string iv;
        m_Crypto.GenerateIv(iv);
        cred.SetIv(iv);
        // Generate Credentials
        //cred = m_Crypto.GenerateCredentials();
        
        pFi->SetCredentials(cred);
    }
    else
    {
        // Use existing Credentials
        cred = pFi->GetCredentialsCopy();
    }

    status = m_Crypto.EncryptFile(comppath, cryptpath, cred);
    if(status != ret::A_OK)
        return status;

    std::cout<< " file encrypted " << std::endl;

    // Shove keys into a sqlite entry (and FileInfo?)

    // ChunkFile
    std::string cn;
    pFi->GetChunkName(cn);
    std::cout<<" <--------------------- CHUNK NAME : " << cn << std::endl;
    // Generate Chunk Directory 
    status = m_Chunker.ChunkFile(pFi, cryptpath, m_TempDirectory);
    if(status != ret::A_OK)
        return status;

    std::cout<< " file chunked " << std::endl;
    // Check if manifest is loaded
    // Write manifest entry

    if(insert)
        m_Manifest.InsertFileInfo(pFi);

    return status;
}

ret::eCode FileManager::RemoveFile(const std::string &filename)
{
    ret::eCode status = ret::A_OK;

    if(!m_Manifest.RemoveFileInfo(filename))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;

    return status;
}

void FileManager::GenerateCompressionPath(FileInfo* pFi, std::string &outpath)
{
    if(!pFi)
        return;

    // strip any file type
    std::vector<std::string> split;
    std::string filename;
    pFi->GetFilename(filename);

    utils::SplitString(filename, '.', split);
    
    if(split.size() > 0)
    { 
        //outpath = m_WorkingDirectory + "/" + split[0] + "_cmp";
        outpath = m_TempDirectory + "/" + split[0] + "_cmp";
    }
}

void FileManager::GenerateCryptoPath(FileInfo* pFi, std::string &outpath)
{
    if(!pFi)
        return;
    // strip any file type
    std::vector<std::string> split;
    std::string filename;
    pFi->GetFilename(filename);

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


int FileManager::CheckManifestForFile(const std::string& filename, FileInfo* pFi)
{
    int status = ret::A_OK;

    if(pFi)
    {
        if(!m_Manifest.QueryForFile(filename, pFi))
        {
            status = ret::A_FAIL_TO_QUERY_MANIFEST;
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::DecryptChunks(FileInfo* pFi)
{

    std::cout<<" Decrypt Chunks ... " << std::endl;
    int status = ret::A_OK;

    if(pFi)
    {
        // Get Credentials
        Credentials cred;
        m_MasterKey.GetMasterKeyCredentials(cred);

        std::string key;
        cred.GetKey(key);
        std::cout<<" THIS KEY : " << key << std::endl;
        
        std::vector<ChunkInfo*>* pInfo = pFi->GetChunkInfoList();
        std::vector<ChunkInfo*>::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {
            if(*itr)
            {
                std::string chunkname;
                (*itr)->GetChunkName(chunkname);

                // Set given iv for chunk
                std::string iv;
                (*itr)->GetIv(iv);

                std::cout<<" THIS IV : " << iv << std::endl;

                status = cred.SetIv(iv);
                if(status != ret::A_OK)
                    break;

                // going from enc to cmp
                std::string encpath;
                GenerateEncryptionPath(chunkname, encpath);

                std::cout<<" enc path : " << encpath << std::endl;

                std::string comppath;
                GenerateCompressionPath(chunkname, comppath);

                status = m_Crypto.DecryptFile(encpath, comppath, cred);
                if(status != ret::A_OK)
                {
                    std::cout<<" Failed to decrypt file code : " << status << std::endl;
                    break;
                }
            }
        }

    }
    else
    {
        std::cout<<" Invalid file info ptr " << std::endl;
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::DecompressChunks(FileInfo* pFi)
{
    int status = ret::A_OK;

    if(pFi)
    {
        std::vector<ChunkInfo*>* pInfo = pFi->GetChunkInfoList();
        std::vector<ChunkInfo*>::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {
            if(*itr)
            {
                std::string chunkname;
                (*itr)->GetChunkName(chunkname);
                
                std::string comppath;
                GenerateCompressionPath(chunkname, comppath);

                std::string chunkpath;
                chunkpath += m_TempDirectory + "/" + chunkname;

                status = m_Compressor.DecompressFile(comppath, chunkpath);

                if(status != ret::A_OK)
                    break;
            }
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::DechunkFile(FileInfo* pFi)
{
    int status = ret::A_OK;

    if(pFi)
    {
        // TODO :: figure out which directory this file actually belongs in,
        //         for now just construct in temp directory

        std::string outpath; // Outbound directory path for fully constructed file
        std::string filename;
        pFi->GetFilename(filename);
        outpath = m_TempDirectory + "/" + filename;

        status = m_Chunker.DeChunkFile(pFi, outpath, m_TempDirectory);
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::ConstructFileNew(const std::string& filename)
{
    // TODO :: this assumes that the fileinfo AND chunkinfo will be successfully created
    //         the chunk info data must be in the fileinfo, so before this, OUTSIDE of this,
    //         pull the meta post and chunk post and do all that. and possibly insert into
    //         manifest
    int status = ret::A_OK;

    // Retrieve File Info from manifest
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();

    status = CheckManifestForFile(filename, pFi);

    std::cout<< " how : " << status << std::endl;

    if(status != ret::A_OK)
        return status;

    // Decrypt Chunks
    std::cout<< " decrypting ... " << std::endl;
    status = DecryptChunks(pFi);
    if(status != ret::A_OK)
        return status;

    // Decompress Chunks
    std::cout<< " decompressing ... " << std::endl;
    status = DecompressChunks(pFi);
    if(status != ret::A_OK)
        return status;

    // Construct File
    std::cout<< " constructing ... " << std::endl;
    status = DechunkFile(pFi);
    if(status != ret::A_OK)
        return status;

    return status;
}

ret::eCode FileManager::ConstructFile(std::string &filename)
{

    // 
    ret::eCode status = ret::A_OK;
    // Retrieve File Info from manifest
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();

    if(!pFi)
        return ret::A_FAIL_INVALID_PTR;

    m_Manifest.QueryForFile(filename, pFi);

    if(!pFi)
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

    status = m_Chunker.DeChunkFile(pFi, chunkpath, m_TempDirectory);

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

    status = m_Crypto.DecryptFile(chunkpath, decrypPath, pFi->GetCredentialsCopy());
    
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
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();

    if(!pFi)
        std::cout<<"INVALID"<<std::endl;
    m_Manifest.QueryForFile(filename, pFi);

    if(!pFi)
        std::cout<<"INVALID"<<std::endl;
    std::cout<<"427"<<std::endl;
    if(!pFi->IsValid())
        return NULL;
    return pFi;
}
