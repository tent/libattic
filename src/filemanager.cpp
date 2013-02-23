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
    pFi->SetEncryptedKey(key);
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

        std::cout<<" ChunkFile Filepath : " << filepath << std::endl;

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
        FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
        FileInfo::ChunkMap::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {

            std::string chunkname;
            itr->second.GetChunkName(chunkname);
            
            std::string comppath;
            GenerateCompressionPath(chunkname, comppath);

            std::string filepath;
            filepath += m_TempDirectory + "/" + chunkname;

            status = m_Compressor.CompressFile(filepath, comppath, 1);

            if(status != ret::A_OK)
                break;

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
        // Get Master Key Credentials
        Credentials masterCred;
        m_MasterKey.GetMasterKeyCredentials(masterCred);

        std::string masterKey;
        masterCred.GetKey(masterKey);

        std::cout<<" MASTER KEY : " << masterKey << std::endl;
        std::cout<<" MASTER KEY size : " << masterKey.size() << std::endl;

        Credentials fileCred;
        if(!pFi->HasEncryptedKey())
        {
            // Generate Credentials
            fileCred = m_Crypto.GenerateCredentials();

            // Encrypt Key
            std::string fKey, fIv, encKey;
            fileCred.GetKey(fKey);
            fileCred.GetIv(fIv);

            Credentials encCred; // Encryption credentials
            status = encCred.SetKey(masterKey);
            encCred.SetIv(fIv);

            std::cout<<" cred status set key : "<< status << std::endl;
            status = m_Crypto.EncryptStringCFB(fKey, encCred, encKey);
 
            // Set Credentials
            pFi->SetEncryptedKey(encKey); // ENCRYPTED key
            pFi->SetIv(fIv); // NON encrypted, iv           

            std::cout<< " ENCRYPTED KEY : " << encKey << std::endl;
            std::cout<< " IV : " << fIv << std::endl;
        }

        std::string fileKey;
        status = GetDecryptedFileKey(pFi, fileKey);

        // If credentials pass
        if(status == ret::A_OK)
        {
            // Get UNENCRYPTED file key
            std::cout<< " UNENCRYPTED FILE KEY : " << fileKey << std::endl;

            Credentials chunkCred;
            chunkCred.SetKey(fileKey);

            FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
            FileInfo::ChunkMap::iterator itr = pInfo->begin();

            for(;itr != pInfo->end(); itr++)
            {
                std::string chunkname;
                itr->second.GetChunkName(chunkname);
                    
                std::string iv;
                // Generate unique Iv for chunk, every time
                m_Crypto.GenerateIv(iv);
                itr->second.SetIv(iv);

                // Set chunk specific iv
                status = chunkCred.SetIv(iv);

                if(status != ret::A_OK)
                    break;

                std::string comppath;
                GenerateCompressionPath(chunkname, comppath);

                std::string encpath;
                GenerateEncryptionPath(chunkname, encpath);
                 
                status = m_Crypto.EncryptFile(comppath, encpath, chunkCred);

                // Testing thing, remove TODO :: 
                std::string kkkk, ivv;
                pFi->GetEncryptedKey(kkkk);
                pFi->GetIv(ivv);
                std::cout<<" ENCRYPTED KEYYY : " << kkkk << std::endl;
                std::cout<<" IV : " << ivv << std::endl;
                /////
            }
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::GenerateHMACForChunks(FileInfo* pFi)
{
    if(!pFi) return ret::A_FAIL_INVALID_PTR;

    int status = ret::A_OK;

    if(pFi->HasEncryptedKey())
    {
        Credentials masterCred;
        m_MasterKey.GetMasterKeyCredentials(masterCred);

        std::string masterKey;
        masterCred.GetKey(masterKey);

        Credentials fileCred;
        fileCred = pFi->GetCredentialsCopy();

        // Decrypt key
        std::string fKey, fIv;
        fileCred.GetKey(fKey);
        fileCred.GetIv(fIv); // File (key) specific iv

        Credentials encCred; // Encryption credentials
        encCred.SetKey(masterKey);
        encCred.SetIv(fIv);

        std::string fileKey; // Decrypted File Key
        status = m_Crypto.DecryptStringCFB(fKey, encCred, fileKey);

        // Set Decrypted Key
        if(status == ret::A_OK)
        {
            Credentials chunkCred;
            chunkCred.SetKey(fileKey);

            FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
            FileInfo::ChunkMap::iterator itr = pInfo->begin();

            for(;itr != pInfo->end(); itr++)
            {
                // TODO :: Note* at this moment the iv may not be
                //         important
                
                // Assemble chunk cred
                std::string iv;
                itr->second.GetIv(iv);
                chunkCred.SetIv(iv);

                // GetChunk name
                std::string chunkname;
                itr->second.GetChunkName(chunkname);
                 
                std::string chunkpath;
                chunkpath = m_TempDirectory + "/" + chunkname;
                // Get Filepath
                //
                //
                //

                std::string mac;
                m_Crypto.GenerateHMACForFile( chunkpath,
                                              chunkCred,
                                              mac);
                itr->second.SetPlainTextMac(mac);

                std::cout<<" PLAINTEXT MAC : " << mac << std::endl;
            }
        }
    }
    else
    {
        status = ret::A_FAIL_NO_CREDENTIALS;
    }

    return status;
}

int FileManager::GenerateHMACForEncryptedChunks(FileInfo* pFi)
{
    if(!pFi) return ret::A_FAIL_INVALID_PTR;

    int status = ret::A_OK;

    if(pFi->HasEncryptedKey())
    {
        Credentials masterCred;
        m_MasterKey.GetMasterKeyCredentials(masterCred);

        std::string masterKey;
        masterCred.GetKey(masterKey);

        Credentials fileCred;
        fileCred = pFi->GetCredentialsCopy();

        // Decrypt key
        std::string fKey, fIv;
        fileCred.GetKey(fKey);
        fileCred.GetIv(fIv); // File (key) specific iv

        Credentials encCred; // Encryption credentials
        encCred.SetKey(masterKey);
        encCred.SetIv(fIv);

        std::string fileKey; // Decrypted File Key
        status = m_Crypto.DecryptStringCFB(fKey, encCred, fileKey);

        // Set Decrypted Key
        if(status == ret::A_OK)
        {
            Credentials chunkCred;
            chunkCred.SetKey(fileKey);

            FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
            FileInfo::ChunkMap::iterator itr = pInfo->begin();

            for(;itr != pInfo->end(); itr++)
            {
                // TODO :: Note* at this moment the iv may not be
                //         important
                
                // Assemble chunk cred
                std::string iv;
                itr->second.GetIv(iv);
                chunkCred.SetIv(iv);

                // GetChunk name
                std::string chunkname;
                itr->second.GetChunkName(chunkname);
                 
                // Get Filepath
                std::string encpath;
                GenerateEncryptionPath(chunkname, encpath);

                std::string mac;
                m_Crypto.GenerateHMACForFile( encpath,
                                              chunkCred,
                                              mac);
                itr->second.SetCipherTextMac(mac);
            }
        }
    }
    else
    {
        status = ret::A_FAIL_NO_CREDENTIALS;
    }

    return status;
}

int FileManager::VerifyHMACForChunks(FileInfo* pFi)
{
    if(!pFi) return ret::A_FAIL_INVALID_PTR;
    int status = ret::A_OK;

    std::string fileKey;
    status = GetDecryptedFileKey(pFi, fileKey);

    if(status == ret::A_OK)
    {
        Credentials chunkCred;
        chunkCred.SetKey(fileKey);

        FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
        FileInfo::ChunkMap::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {
            // Assemble chunk cred
            std::string iv;
            itr->second.GetIv(iv);
            chunkCred.SetIv(iv);

            // GetChunk name
            std::string chunkname;
            itr->second.GetChunkName(chunkname);

            // GetCipherTextHmac
            std::string mac;
            itr->second.GetPlainTextMac(mac);
             
            // Get Filepath
            std::string chunkpath;
            chunkpath = m_TempDirectory + "/" + chunkname;

            m_Crypto.VerifyHMACForFile( chunkpath,
                                        chunkCred,
                                        mac);
        }
    }


    return status;
}

int FileManager::VerifyHMACForEncryptedChunks(FileInfo* pFi)
{
    if(!pFi) return ret::A_FAIL_INVALID_PTR;
    int status = ret::A_OK;

    std::string fileKey;
    status = GetDecryptedFileKey(pFi, fileKey);

    if(status == ret::A_OK)
    {
        Credentials chunkCred;
        chunkCred.SetKey(fileKey);

        FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
        FileInfo::ChunkMap::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {
            // Assemble chunk cred
            std::string iv;
            itr->second.GetIv(iv);
            chunkCred.SetIv(iv);

            // GetChunk name
            std::string chunkname;
            itr->second.GetChunkName(chunkname);

            // GetCipherTextHmac
            std::string mac;
            itr->second.GetCipherTextMac(mac);
             
            // Get Filepath
            std::string encpath;
            GenerateEncryptionPath(chunkname, encpath);

            m_Crypto.VerifyHMACForFile( encpath,
                                        chunkCred,
                                        mac);
        }
    }
 
    return status;
}

int FileManager::GetDecryptedFileKey(FileInfo* pFi, std::string& keyOut)
{
    if(!pFi) return ret::A_FAIL_INVALID_PTR;
    int status = ret::A_OK;

    if(pFi->HasEncryptedKey())
    {
        Credentials masterCred;
        m_MasterKey.GetMasterKeyCredentials(masterCred);

        std::string masterKey;
        masterCred.GetKey(masterKey);

        Credentials fileCred;
        fileCred = pFi->GetCredentialsCopy();

        // Decrypt key
        std::string fKey, fIv;
        fileCred.GetKey(fKey);
        fileCred.GetIv(fIv); // File (key) specific iv

        Credentials encCred; // Encryption credentials
        encCred.SetKey(masterKey);
        encCred.SetIv(fIv);

        std::string fileKey; // Decrypted File Key
        status = m_Crypto.DecryptStringCFB(fKey, encCred, fileKey);

        if(status == ret::A_OK)
            keyOut = fileKey;
    }
    else
    {
        status = ret::A_FAIL_INVALID_FILE_KEY;
    }

    return status;
}

int FileManager::IndexFileNew( const std::string& filepath,
                               const bool insert,
                               FileInfo* pFi)
{
    // FileInfo ptr check down further, don't add please.
    
    // Chunk
    // Compress
    // Encrpyt
    // Post
    
    // TODO :: Handle re-indexing, a file was edited, or the temporary folder was deleted
    std::cout << "Indexing file ... " << std::endl;
    int status = ret::A_OK;

    if(!m_MasterKey.IsEmpty())
    {
        status = ret::A_FAIL_INVALID_MASTERKEY;
        std::cout<<" EMPTY master key ... " <<std::endl;
    }
   
    if(status == ret::A_OK)
    {
        // Create an entry
        //  Get File info
        bool reindex = true;
        if(!pFi)
        {
            pFi = CreateFileInfo();
            pFi->InitializeFile(filepath);
            reindex = false;
        }

        std::string tp;
        pFi->GetFilepath(tp);
 
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

        status = GenerateHMACForChunks(pFi);
        if(status != ret::A_OK)
            return status;

        status = GenerateHMACForEncryptedChunks(pFi);
        if(status != ret::A_OK)
            return status;

        // Shove keys into a sqlite entry (and FileInfo?)

        // Check if manifest is loaded
        // Write manifest entry
        if(insert)
        {
            // Encrypt File key
            std::string fileKey;
            pFi->GetEncryptedKey(fileKey);
            m_Manifest.InsertFileInfo(pFi);
        }

        std::string jsontest;
        pFi->GetSerializedChunkData(jsontest);
        pFi->LoadSerializedChunkData(jsontest);
    }

    return status;
}

int FileManager::RemoveFile(const std::string &filepath)
{
    int status = ret::A_OK;

    if(!m_Manifest.RemoveFileInfo(filepath))
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

void FileManager::SetFilePostId(const std::string &filepath, const std::string& postid)
{
    m_Manifest.InsertFilePostID(filepath, postid);
}

void FileManager::SetFileChunkPostId(const std::string &filepath, const std::string& postid)
{
    m_Manifest.InsertFileChunkPostID(filepath, postid);
}

void FileManager::GetFilePostId(const std::string& filename, std::string& out)
{

}

void FileManager::GetFileChunkPostId(const std::string& filename, std::string& out)
{

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
        // Get MasterKey
        std::string masterKey;
        m_MasterKey.GetMasterKey(masterKey);

        // Get Credentials

        // Get Encrypted File Credentials
        std::string encKey, fileIv;
        pFi->GetEncryptedKey(encKey);
        pFi->GetIv(fileIv);

        std::cout<<" MASTER KEY : " << masterKey << std::endl;
        std::cout<<" ENCRYPTED KEEEEEEEY : " << encKey << std::endl;
        std::cout<<" FILE IV : " << fileIv << std::endl;

        Credentials cred;
        cred.SetKey(masterKey);
        cred.SetIv(fileIv);

        std::string fileKey;
        status = GetDecryptedFileKey(pFi, fileKey);

        std::cout<<" STATUS : " << status << std::endl;
        std::cout<<" FILE KEY : " << fileKey << std::endl;

        // TODO :: Finish this, left in a bad state possibly, test
        if(status == ret::A_OK)
        {
            Credentials chunkCred;
            chunkCred.SetKey(fileKey);

            // Decrypt Key
            std::string key;
            cred.GetKey(key);
            std::cout<<" THIS KEY : " << key << std::endl;
            
            FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
            FileInfo::ChunkMap::iterator itr = pInfo->begin();

            for(;itr != pInfo->end(); itr++)
            {
                std::string chunkname;
                itr->second.GetChunkName(chunkname);

                // Set given iv for chunk
                std::string iv;
                itr->second.GetIv(iv);

                std::cout<<" THIS IV : " << iv << std::endl;

                status = chunkCred.SetIv(iv);
                if(status != ret::A_OK)
                    break;

                // going from enc to cmp
                std::string encpath;
                GenerateEncryptionPath(chunkname, encpath);

                std::cout<<" enc path : " << encpath << std::endl;

                std::string comppath;
                GenerateCompressionPath(chunkname, comppath);

                status = m_Crypto.DecryptFile(encpath, comppath, chunkCred);
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
        FileInfo::ChunkMap* pInfo = pFi->GetChunkInfoList();
        FileInfo::ChunkMap::iterator itr = pInfo->begin();

        for(;itr != pInfo->end(); itr++)
        {
            std::string chunkname;
            itr->second.GetChunkName(chunkname);
            
            std::string comppath;
            GenerateCompressionPath(chunkname, comppath);

            std::string chunkpath;
            chunkpath += m_TempDirectory + "/" + chunkname;

            status = m_Compressor.DecompressFile(comppath, chunkpath);

            if(status != ret::A_OK)
                break;
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
        std::string filepath;
        pFi->GetFilename(filename);
        pFi->GetFilepath(filepath);

        outpath = m_TempDirectory + "/" + filename;

        //status = m_Chunker.DeChunkFile(pFi, outpath, m_TempDirectory);
        //
        status = m_Chunker.DeChunkFile(pFi, filepath, m_TempDirectory);
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int FileManager::ConstructFileNew(const std::string& filepath)
{
    // TODO :: this assumes that the fileinfo AND chunkinfo will be successfully created
    //         the chunk info data must be in the fileinfo, so before this, OUTSIDE of this,
    //         pull the meta post and chunk post and do all that. and possibly insert into
    //         manifest
    int status = ret::A_OK;

    // Retrieve File Info from manifest
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();

    status = CheckManifestForFile(filepath, pFi);

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
    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    Unlock();

    return pFi;
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
    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    m_Manifest.QueryForFile(filename, pFi);
    Unlock();

    if(pFi) {
        if(!pFi->IsValid())
            return NULL;
    }

    return pFi;
}

int FileManager::GetAllFileInfo(std::vector<FileInfo>& out)
{
    return m_Manifest.QueryAllFiles(out);
}

