#include "postfilestrategy.h"

#include <stdio.h>

#include "utils.h"
#include "compression.h"
#include "constants.h"
#include "fileinfo.h"
#include "filesystem.h"
#include "credentialsmanager.h"
#include "chunkpost.h"
#include "rollsum.h"

PostFileStrategy::PostFileStrategy() {
    m_pCredentialsManager = NULL;
    m_pFileManager = NULL;
}

PostFileStrategy::~PostFileStrategy() {
    m_pCredentialsManager = NULL;
    m_pFileManager = NULL;
}

void PostFileStrategy::Execute(FileManager* pFileManager,
                               CredentialsManager* pCredentialsManager,
                               const std::string& entityApiRoot, 
                               const std::string& filepath, 
                               Response& out)
{
    int status = ret::A_OK;
    m_entityApiRoot = entityApiRoot;
    m_pFileManager = pFileManager;
    m_pCredentialsManager = pCredentialsManager;
    if(!m_pFileManager) return;
    if(!m_pCredentialsManager) return;
    m_pCredentialsManager->GetAccessTokenCopy(m_At);

    // Verify file exists
    if(fs::CheckFileExists(filepath)) {
        // Begin the chunking pipeline
        FileInfo* fi = RetrieveFileInfo(filepath);

        std::string requesttype, posturl, chunkpostid;
        Credentials fileCredentials;
        status = DetermineChunkPostRequest(fi, fileCredentials, requesttype, posturl, chunkpostid);
       
        // Check for key
        if(fileCredentials.KeyEmpty() || fileCredentials.IvEmpty()) {
            std::cout<<"FAIL " << std::endl;
            status = ret::A_FAIL_INVALID_FILE_KEY;
        }

        Response resp;
        if(status == ret::A_OK) {
            status = ProcessFile( requesttype,
                                  posturl,
                                  filepath,
                                  fileCredentials,
                                  fi,
                                  resp);
        }

        out = resp;
    }
}

int PostFileStrategy::DetermineChunkPostRequest(FileInfo* fi, 
                                                Credentials& credOut, 
                                                std::string& requestTypeOut,
                                                std::string& urlOut,
                                                std::string& postidOut)
{
    int status = ret::A_OK;

    postidOut.clear();
    // Construct Post url
    urlOut = m_entityApiRoot;
    utils::CheckUrlAndAppendTrailingSlash(urlOut);
    urlOut += "posts";
    
    fi->GetChunkPostID(postidOut);

    // Initiate Chunk Post request
    if(postidOut.empty()) {
        requestTypeOut = "POST";
        // This is a new file
        // Generate Credentials
        crypto::GenerateCredentials(credOut);
    }
    else {
        requestTypeOut = "PUT";
        utils::CheckUrlAndAppendTrailingSlash(urlOut);
        urlOut += postidOut;
        std::string encryptedkey, fileiv; 

        fi->GetEncryptedKey(encryptedkey);
        fi->GetIv(fileiv);
        // Decrypt file key
        Credentials masterCred;
        MasterKey mKey;
        m_pCredentialsManager->GetMasterKeyCopy(mKey);
        std::string mk;
        mKey.GetMasterKey(mk);
        masterCred.SetKey(mk);
        masterCred.SetIv(fileiv);

        std::string decryptedkey;
        crypto::DecryptStringCFB(encryptedkey, masterCred, decryptedkey);
        fi->SetFileKey(decryptedkey);
        fi->GetFileCredentials(credOut);
    }
    return status;
}

int PostFileStrategy::ProcessFile(const std::string& requestType,
                                  const std::string& url,
                                  const std::string& filepath,
                                  const Credentials& fileCredentials,
                                  FileInfo* pFi,
                                  Response& resp)
{
    int status = ret::A_OK;

    std::cout<< "processing file ... " <<std::endl;
    std::cout<< "filepath : " << filepath << std::endl;

    std::string protocol, host, path;
    netlib::ExtractHostAndPath(url, protocol, host, path);
            
    boost::asio::io_service io_service; 
    tcp::socket socket(io_service); 
            
    status = netlib::ResolveHost(io_service, socket, host); 
    std::cout<<" Resolve host : " << status << std::endl;
    std::cout<<" protocol : " << protocol << std::endl;
    std::cout<<" host : " << host << std::endl;
    std::cout<<" path : " << path << std::endl;
    std::cout<<" connection status : " << status << std::endl;
    if(status == ret::A_OK) {
        // Setup SSL handshake
        boost::system::error_code error = boost::asio::error::host_not_found; 
        // setup an ssl context 
        boost::asio::ssl::context ctx( io_service, 
                                       boost::asio::ssl::context::sslv23_client); 
        ctx.set_verify_mode(boost::asio::ssl::context::verify_none);
        boost::asio::ssl::stream<tcp::socket&> ssl_sock(socket, ctx);

        ssl_sock.handshake(boost::asio::ssl::stream_base::client, error);
        if (error) {
            alog::Log( Logger::ERROR, 
                       boost::system::system_error(error).what(), 
                       ret::A_FAIL_SSL_HANDSHAKE);
            status = ret::A_FAIL_SSL_HANDSHAKE;
        }
        if(status != ret::A_OK)
            return status;

        std::string boundary;
        utils::GenerateRandomString(boundary, 20);

        std::string fileKey;
        fileCredentials.GetKey(fileKey);

        // Build request
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        netlib::BuildRequestHeader(requestType, url, boundary, &m_At, request_stream); 

        // Build Body Form header
        ChunkPost p;
        std::string body; // we send an empty body for now
        jsn::SerializeObject(&p, body);

        boost::asio::streambuf requestBody;
        std::ostream part_stream(&requestBody);
        netlib::BuildBodyForm(body, boundary, part_stream);

        // Chunk the Body 
        boost::asio::streambuf chunkedBody;
        std::ostream partbuf(&chunkedBody);
        netlib::ChunkPart(requestBody, partbuf);

        // Start the request
        boost::asio::write(ssl_sock, request); 
        boost::asio::write(ssl_sock, chunkedBody);

        const unsigned int filesize = utils::CheckFilesize(filepath);
        // start the process
        std::cout<<" attempting to open filepath : " << filepath << std::endl;
        std::ifstream ifs;
        ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

        unsigned int count = 0;
        unsigned int totalread = 0; // total read count
        if (ifs.is_open()) {
            // Setup window buffer
            std::string window, remainder;
            unsigned int readcount = 0;
            unsigned int chunkcount = 0;
            RollSum rs;
            while(!ifs.eof()) {
                char* pData = NULL;
                unsigned int datasize = 0;
                if(filesize >= cnst::g_unMaxBuffer) {
                    pData = new char[filesize];
                    datasize = filesize;
                }
                else {
                    if((filesize - readcount) <= cnst::g_unMaxBuffer) {
                        pData = new char[(filesize - readcount)];
                        datasize = (filesize - readcount);
                    }
                    else {
                        pData = new char[cnst::g_unMaxBuffer];
                        datasize = cnst::g_unMaxBuffer;
                    }
                }

                ifs.read(pData, datasize);
                readcount += datasize;

                window.clear();
                window += remainder;
                remainder.clear();

                if(datasize)
                    window.append(pData, datasize);

                if(pData) {
                    delete[] pData;
                    pData = NULL;
                }

                // find splits
                int totalread = 0, count = 0, lastsplit = 0;
                for(unsigned int i=0; i<window.size(); i++) {
                    char c = window[i];
                    count++;
                    rs.Roll(c);
                    if(rs.OnSplit()) {
                        if(count >= cnst::g_unSplitMin) {
                            std::string chunk;
                            int diff = i - lastsplit;
                            chunk = window.substr(lastsplit, diff);
                            // Transform
                            SendChunk( chunk, fileKey, boundary, ssl_sock, count, false, pFi);
                            lastsplit = i;
                            count = 0;
                            chunkcount++;
                            totalread += chunk.size();
                        }
                    }

                    if(count >= cnst::g_unSplitMax) {
                        std::string chunk;
                        int diff = i - lastsplit;
                        chunk = window.substr(lastsplit, diff);
                        // Transform
                        SendChunk( chunk, fileKey, boundary, ssl_sock, count, false, pFi);
                        lastsplit = i;
                        count = 0;
                        chunkcount++;
                        totalread += chunk.size();
                    }
                }

                // Set remainder
                int wdiff = window.size() - totalread;
                if(wdiff)
                    remainder = window.substr(lastsplit, wdiff);

                if((readcount + remainder.size()) >= filesize) {
                    SendChunk( remainder, fileKey, boundary, ssl_sock, chunkcount, true, pFi);
                    chunkcount++;
                    break;
                }
            }
            
            boost::asio::streambuf response;
            boost::asio::read_until(ssl_sock, response, "\r\n");
            std::string responseheaders;
            netlib::InterpretResponse(response, ssl_sock, resp, responseheaders);
        }
        else {
            status = ret::A_FAIL_OPEN_FILE;
        }
    }

    if(resp.code != 200) { 
        std::cout<<" FAILED TO CONNECT : " << resp.code << std::endl;
        std::cout<<" body : " << resp.body << std::endl;
        std::cout<<" status : " << status << std::endl;
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int PostFileStrategy::SendChunk( const std::string& chunk, 
                         const std::string& fileKey,
                         const std::string& boundary,
                         boost::asio::ssl::stream<tcp::socket&>& ssl_sock,
                         const unsigned int count,
                         bool end,
                         FileInfo* pFi)
{
    int status = ret::A_OK;
    // Transform
    std::string finishedChunk, chunkName;
    TransformChunk( chunk, 
                    fileKey, 
                    finishedChunk, 
                    chunkName, 
                    pFi);

    // Build Attachment
    boost::asio::streambuf attachment;
    std::ostream attachmentstream(&attachment);
    //netlib::BuildAttachmentForm(chunkName, finishedChunk, boundary, count, attachmentstream);
    netlib::BuildAttachmentForm(chunkName, finishedChunk, boundary, count, attachmentstream);

    // create multipart post
    if(end) {
        // Add end part
        netlib::AddEndBoundry(attachmentstream, boundary);
        std::cout<<" CHUNK END PART " << std::endl;
        // Chunk the end
        boost::asio::streambuf partEnd;
        std::ostream partendstream(&partEnd);
        netlib::ChunkEnd(attachment, partendstream);
        netlib::WriteToSSLSocket(ssl_sock, partEnd);
    }
    else {
        // carry on
        // Chunk the part
        boost::asio::streambuf part;
        std::ostream chunkpartbuf(&part);
        netlib::ChunkPart(attachment, chunkpartbuf);
        netlib::WriteToSSLSocket(ssl_sock, part);
    }

    return status;
}

int PostFileStrategy::TransformChunk(const std::string& chunk, 
                                     const std::string& fileKey,
                                     std::string& finalizedOut, 
                                     std::string& nameOut, 
                                     FileInfo* pFi)
{
    int status = ret::A_OK;

    Credentials chunkCred;
    chunkCred.SetKey(fileKey);

    // Calculate plaintext hash
    std::string plaintextHash;
    crypto::GenerateHash(chunk, plaintextHash);

    // create chunk name (hex encoded plaintext hash)
    std::string chunkName;
    utils::StringToHex(plaintextHash, chunkName);

    // Compress
    std::string compressedChunk;
    compress::CompressString(chunk, compressedChunk);

    // Encrypt
    std::string encryptedChunk;
    std::string iv;
    crypto::GenerateIv(iv);
    chunkCred.SetIv(iv);

    std::cout<< " IV : " << iv << std::endl;
    crypto::EncryptStringCFB(compressedChunk, chunkCred, encryptedChunk);
    //crypto::EncryptStringGCM(compressedChunk, chunkCred, encryptedChunk);

    // Base64 Encode
    std::string finishedChunk;
    crypto::Base64EncodeString(encryptedChunk, finishedChunk);
    finalizedOut = finishedChunk;

    std::string ciphertextHash;
    crypto::GenerateHash(encryptedChunk, ciphertextHash);

    // Fill Out Chunk info object
    ChunkInfo ci;
    ci.SetChunkName(chunkName);
    ci.SetPlainTextMac(plaintextHash);
    ci.SetCipherTextMac(ciphertextHash);
    ci.SetIv(iv);

    nameOut = chunkName;

    // Push chunk back into fileinfo
    if(pFi) pFi->PushChunkBack(ci);

    return status;
}

FileInfo* PostFileStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = m_pFileManager->GetFileInfo(filepath);
    if(!fi)
        fi = m_pFileManager->CreateFileInfo();
    return fi;
}

