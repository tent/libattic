
#include "filemanager.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

#include <zlib.h>
#include <gzip.h>
#include <files.h>
#include <mqueue.h>
#include <channels.h>

#include "utils.h"

// TODO :: considering removing all file reading/writing out of 
//         manifest to filemanager. (centralize all file writing)
//

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

    return LoadManifest(m_ManifestFilePath);
}

bool FileManager::ShutdownFileManager()
{
    // Write out manifest
    return m_Manifest.WriteOutManifest();
}

bool FileManager::LoadManifest(std::string &szFilePath)
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
                m_ifStream.close();

                return false;
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
    if(split.size() < 5)
        return false;

    FileInfo* fi = m_FileInfoFactory.CreateFileInfoObject();

    // Filename (str)
    fi->SetFileName(split[0]);
    // Filepath (str)
    fi->SetFilePath(split[1]);
    // ChunkName (str)
    fi->SetChunkName(split[2]);
    // ChunkCount (unsigned int)
    fi->SetChunkCount((unsigned)atoi(split[3].c_str()));
    // FileSize (unsigned int)
    fi->SetFileSize((unsigned)atoi(split[4].c_str()));

    return true;
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
     
    // ChunkFile
    unsigned int count = m_Chunker.ChunkFile(fi);
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

bool FileManager::ConstructFile(std::string &szFileName)
{
    // Retrieve File Info from manifest
    FileInfo *pFi = m_Manifest.RetrieveFileInfo(szFileName);
    if(!pFi)
        return false;

    // Decrypt chunks
    //
    // De-chunk
    if(! m_Chunker.DeChunkFile(pFi))
        return false;
    // Decompress
    //

    return true;
}

FileInfo* FileManager::CreateFileInfo()
{
    return m_FileInfoFactory.CreateFileInfoObject();
}

bool FileManager::FileExists(std::string& szFilepath)          
{                                                           
        std::ifstream infile(szFilepath.c_str());               
        bool bVal = infile.good();
        infile.close();
        return bVal;                               
}                                                           

bool FileManager::CompressFile(std::string &szFilePath, std::string &szOutputPath, int nDeflatedLevel)
{
    // Pass in path to file needing compressing, output path is the compressed file's destination
    // deflate level (1-9) level of compression

    // TODO :: it'd be nice for some sort of filepath validation.
    //

    try // Cryptopp can throw exceptions
    {
        // Use filter to check integrity after compression    
        CryptoPP::EqualityComparisonFilter comparison;
        CryptoPP::ChannelSwitch *compSwitch = new CryptoPP::ChannelSwitch(comparison, "0");
        // gunzip takes ownership of the data, and will free it later.
        CryptoPP::Gunzip gz(compSwitch);
        gz.SetAutoSignalPropagation(0);
        
        CryptoPP::FileSink sink(szOutputPath.c_str());
        
        CryptoPP::ChannelSwitch *cs = new CryptoPP::ChannelSwitch(sink);
        // gzip takes ownership of the data, and will free it later.
        CryptoPP::Gzip gzip(cs, nDeflatedLevel);
        cs->AddDefaultRoute(gz);
        
        cs = new CryptoPP::ChannelSwitch(gzip);
        cs->AddDefaultRoute(comparison, "1");

        CryptoPP::FileSource source(szFilePath.c_str(), true, cs);
       
        comparison.ChannelMessageSeriesEnd("0");
        comparison.ChannelMessageSeriesEnd("1");
    }
    catch(std::exception &e)
    {
        // Some sort of logging
        return false;
    }
    
    return true;
}


bool FileManager::DecompressFile(std::string &szFilePath, std::string &szOutputPath)
{
    try
    {
        CryptoPP::FileSource(szFilePath.c_str(), true, new CryptoPP::Gunzip(new CryptoPP::FileSink(szOutputPath.c_str())));
    }
    catch(std::exception &e)
    {
        return false;
    }

    return true;
}



