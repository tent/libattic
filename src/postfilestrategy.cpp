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
#include "postutils.h"

PostFileStrategy::PostFileStrategy() {}
PostFileStrategy::~PostFileStrategy() {}

int PostFileStrategy::Execute(FileManager* pFileManager,
                               CredentialsManager* pCredentialsManager,
                               Response& out){
    int status = ret::A_OK;
    m_pFileManager = pFileManager;
    m_pCredentialsManager = pCredentialsManager;
    if(!m_pFileManager) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    if(!m_pCredentialsManager) return ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    m_pCredentialsManager->GetAccessTokenCopy(m_At);

    m_entityApiRoot = GetConfigValue("api_root");
    std::string filepath = GetConfigValue("filepath");


    // Verify file exists
    if(fs::CheckFileExists(filepath)) {
        // Begin the chunking pipeline
        FileInfo* fi = RetrieveFileInfo(filepath);
        if(!fi) std::cout<<"INVALID FILE INFO " << std::endl;

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

        if(status == ret::A_OK && resp.code == 200) {
            // On success 
            FileInfo::ChunkMap* pList = fi->GetChunkInfoList();
            ChunkPost p;
            // Deserialize basic post data
            jsn::DeserializeObject((Post*)&p, resp.body);
            p.SetChunkInfoList(*pList);

            std::cout<<" CURRENT CHUNK LIST SIZE : " << pList->size() << std::endl;

            // Setup post url
            if(chunkpostid.empty()) {
                chunkpostid = p.id();
                if(chunkpostid.empty()) {
                    status = ret::A_FAIL_INVALID_POST_ID;
                }
                utils::CheckUrlAndAppendTrailingSlash(posturl);
                posturl += chunkpostid;
            }

            if(status == ret::A_OK) {
                // update chunk post with chunk info metadata
                // use non multipart to just update the post body
                // leaving existing attachment in-tact
                std::string bodyBuffer;
                jsn::SerializeObject(&p, bodyBuffer);
    
                std::cout<<" Updating chunk post metadata : " << posturl << std::endl;
                Response metaResp;
                status = netlib::HttpPut(posturl,
                                         p.type(),
                                         NULL,
                                         bodyBuffer,
                                         &m_At,
                                         metaResp);

                std::cout<< " META RESPONSE CODE : " << metaResp.code << std::endl;
                std::cout<< " META RESPONSE BODY : " << metaResp.body << std::endl;

                if(metaResp.code == 200) {
                    out = metaResp;
                    UpdateFileInfo(fileCredentials, filepath, chunkpostid, fi);

                    /*
                    std::string filename;
                    utils::ExtractFileName(filepath, filename);
                    unsigned int filesize = utils::CheckFilesize(filepath);

                    // Encrypt File Key
                    MasterKey mKey;
                    m_pCredentialsManager->GetMasterKeyCopy(mKey);

                    std::string mk;
                    mKey.GetMasterKey(mk);

                    // Insert File Data
                    fi->SetChunkPostID(chunkpostid);
                    fi->SetFilepath(filepath);
                    fi->SetFilename(filename);
                    fi->SetFileSize(filesize);
                    fi->SetFileCredentials(fileCredentials);

                    // Encrypt File Key
                    std::string fileKey, fileIv;
                    fileCredentials.GetKey(fileKey);
                    fileCredentials.GetIv(fileIv);

                    Credentials fCred;
                    fCred.SetKey(mk);
                    fCred.SetIv(fileIv);

                    std::string encryptedKey;
                    crypto::EncryptStringCFB(fileKey, fCred, encryptedKey);

                    fi->SetIv(fileIv);
                    fi->SetFileKey(fileKey);
                    fi->SetEncryptedKey(encryptedKey);

                    // Insert file info to manifest
                    m_pFileManager->InsertToManifest(fi);
                    */
                }
                else {
                    status = ret::A_FAIL_NON_200;
                }
            }
        }
    }
    return status;
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
    postutils::ConstructPostUrl(m_entityApiRoot, urlOut);
    
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
        masterCred.set_key(mk);
        masterCred.set_iv(fileiv);

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
            status = ret::A_FAIL_SSL_HANDSHAKE;
        }
        if(status != ret::A_OK)
            return status;

        std::string boundary;
        utils::GenerateRandomString(boundary, 20);

        std::string fileKey = fileCredentials.key();

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
            
            std::cout<<" interpreting response " << std::endl;
            //boost::asio::streambuf response;
            //boost::asio::read_until(ssl_sock, response, "\r\n");
            //std::string responseheaders;
            //netlib::InterpretResponse(response, ssl_sock, resp, responseheaders);
            netlib::InterpretResponse(&ssl_sock, resp);
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

int PostFileStrategy::SendChunk(const std::string& chunk, 
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
    TransformChunk(chunk, 
                   fileKey, 
                   finishedChunk, 
                   chunkName, 
                   pFi);

    // Build Attachment
    boost::asio::streambuf attachment;
    std::ostream attachmentstream(&attachment);
    //netlib::BuildAttachmentForm(chunkName, finishedChunk, boundary, count, attachmentstream);
    netlib::BuildAttachmentForm(chunkName, finishedChunk, boundary, count, attachmentstream);

    int breakcount = 0;
    int retrycount = 20;

    // create multipart post
    if(end) {
        // Add end part
        netlib::AddEndBoundry(attachmentstream, boundary);
        std::cout<<" CHUNK END PART " << std::endl;
        // Chunk the end
        boost::asio::streambuf partEnd;
        std::ostream partendstream(&partEnd);
        netlib::ChunkEnd(attachment, partendstream);
        //netlib::WriteToSSLSocket(ssl_sock, partEnd);
        //
        WriteToSocket(ssl_sock, partEnd);
    }
    else {
        // carry on
        // Chunk the part
        boost::asio::streambuf part;
        std::ostream chunkpartbuf(&part);
        netlib::ChunkPart(attachment, chunkpartbuf);
        //netlib::WriteToSSLSocket(ssl_sock, part);
        WriteToSocket(ssl_sock, part);
    }


    return status;
}
int PostFileStrategy::WriteToSocket(boost::asio::ssl::stream<tcp::socket&>& ssl_sock, 
                                    boost::asio::streambuf& buffer)
{
    int breakcount = 0;
    int retrycount = 20;

    int status = ret::A_OK;
    unsigned int buffersize = buffer.size();
    boost::system::error_code errorcode;
    do {
        boost::timer::cpu_timer::cpu_timer t;
        size_t byteswritten = boost::asio::write(ssl_sock, buffer, errorcode); 
        std::cout<<" bytes written : " << byteswritten << std::endl;
        if(errorcode) {
            std::cout<<" WRITE ERROR : " << std::endl;
            std::cout<<errorcode.message()<<std::endl;
        }
        else{
            boost::timer::cpu_times time = t.elapsed();
            long elapsed = time.user;
            // To milliseconds
            elapsed *= 0.000001; 
            std::cout<<" ELAPSED : " << elapsed << std::endl;
            if(elapsed > 0) {
                unsigned int bps = (buffersize/elapsed);
                // Raise event
                char szSpeed[256] = {'\0'};
                snprintf(szSpeed, 256, "%u", bps);
                event::RaiseEvent(event::Event::UPLOAD_SPEED, std::string(szSpeed), NULL);
            }
            break;
        }
        if(breakcount > retrycount)
            break;
        breakcount++;
    }while(errorcode);

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
    chunkCred.set_key(fileKey);

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
    chunkCred.set_iv(iv);

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

void PostFileStrategy::UpdateFileInfo(const Credentials& fileCred, 
                                      const std::string& filepath, 
                                      const std::string& chunkpostid,
                                      FileInfo* fi) 
{
    std::string filename;
    utils::ExtractFileName(filepath, filename);
    unsigned int filesize = utils::CheckFilesize(filepath);

    // Encrypt File Key
    MasterKey mKey;
    m_pCredentialsManager->GetMasterKeyCopy(mKey);

    std::string mk;
    mKey.GetMasterKey(mk);

    // Insert File Data
    fi->SetChunkPostID(chunkpostid);
    fi->SetFilepath(filepath);
    fi->SetFilename(filename);
    fi->SetFileSize(filesize);
    fi->SetFileCredentials(fileCred);

    // Encrypt File Key
    std::string fileKey = fileCred.key();
    std::string fileIv = fileCred.iv();

    Credentials fCred;
    fCred.set_key(mk);
    fCred.set_iv(fileIv);

    std::string encryptedKey;
    crypto::EncryptStringCFB(fileKey, fCred, encryptedKey);

    fi->SetIv(fileIv);
    fi->SetFileKey(fileKey);
    fi->SetEncryptedKey(encryptedKey);

    // Insert file info to manifest
    m_pFileManager->InsertToManifest(fi);
}

FileInfo* PostFileStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = m_pFileManager->GetFileInfo(filepath);
    if(!fi)
        fi = m_pFileManager->CreateFileInfo();
    return fi;
}

