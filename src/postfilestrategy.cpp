#include "postfilestrategy.h"

#include <stdio.h>

#include "utils.h"
#include "compression.h"
#include "constants.h"
#include "fileinfo.h"
#include "filesystem.h"
#include "credentialsmanager.h"

#include "rollsum.h"
#include "postutils.h"
#include "logutils.h"

#include "sleep.h"

namespace attic { 

PostFileStrategy::PostFileStrategy() {}
PostFileStrategy::~PostFileStrategy() {}

int PostFileStrategy::Execute(FileManager* pFileManager,
                               CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);

    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filepath = GetConfigValue("filepath");

    std::cout<<" POST PATH : " << post_path_ << std::endl;
    std::cout<<" FILE PATH : " << filepath << std::endl;

    // Verify file exists
    if(fs::CheckFilepathExists(filepath)) {
        // Begin the chunking pipeline
        FileInfo* fi = RetrieveFileInfo(filepath);
        if(!fi) std::cout<<"INVALID FILE INFO " << std::endl;

        std::string requesttype, posturl, chunkpostid;
        Credentials fileCredentials;
        status = DetermineChunkPostRequest(fi, fileCredentials, requesttype, posturl, chunkpostid);
       
        if(status == ret::A_OK) {
            // Check for key
            if(fileCredentials.KeyEmpty() || fileCredentials.IvEmpty()) {
                std::cout<<"FAIL FILE CREDENTIALS EMPTY" << std::endl;
                status = ret::A_FAIL_INVALID_FILE_KEY;
            }
        }
        else { 
            std::cout<<"INVALID FILE CREDENTIALS " << std::endl;
        }

        Response resp;
        if(status == ret::A_OK) {
            status = ProcessFile(requesttype,
                                 posturl,
                                 filepath,
                                 fileCredentials,
                                 fi,
                                 resp);
        }

        if(status == ret::A_OK && resp.code == 200) {
            // On success 
            status = UpdateChunkPostMetadata(fi, resp, fileCredentials);
        }
        else if(resp.code!= 200) {
            log::LogHttpResponse("FORTY333", resp);
        }
    }
    return status;
}

int PostFileStrategy::UpdateChunkPostMetadata(FileInfo* fi, 
                                              const Response& resp,
                                              const Credentials& file_cred) {
    int status = ret::A_OK;
    //FileInfo::ChunkMap* pList = fi->GetChunkInfoList();
    ChunkPost p;
    // Deserialize basic post data
    jsn::DeserializeObject((Post*)&p, resp.body);

    // Setup post url
    std::string post_url;
    std::string chunk_post_id = p.id();

    std::cout<<" CHUNK COUNT : " << fi->GetChunkInfoList()->size();

    std::string filepath = GetConfigValue("filepath");
    UpdateFileInfo(file_cred, filepath, chunk_post_id, p.version_id(), fi);

    return status;
}

int PostFileStrategy::DetermineChunkPostRequest(FileInfo* fi, 
                                                Credentials& credOut, 
                                                std::string& requestTypeOut,
                                                std::string& urlOut,
                                                std::string& postidOut) {
    int status = ret::A_OK;

    // check for post id
    postidOut.clear();
    postidOut = fi->chunk_post_id();

    // Initiate Chunk Post request
    if(postidOut.empty()) {
        requestTypeOut = "POST";
        urlOut = posts_feed_;
        // This is a new file
        // Generate Credentials

        std::cout<<" GENTERATING CREDENTIALS " << std::endl;
        crypto::GenerateCredentials(credOut);
    }
    else {
        std::cout<<" putting ... " << std::endl;
        requestTypeOut = "PUT";
        utils::FindAndReplace(post_path_, "{post}", postidOut, urlOut);

        std::cout<<" url out : " << urlOut << std::endl;
       
        std::string encryptedkey =  fi->encrypted_key();
        std::string fileiv = fi->file_credentials_iv();
        // Decrypt file key
        Credentials masterCred;
        MasterKey mKey;
        credentials_manager_->GetMasterKeyCopy(mKey);
        std::string mk;
        mKey.GetMasterKey(mk);
        masterCred.set_key(mk);
        masterCred.set_iv(fileiv);

        std::string decryptedkey;
        //crypto::DecryptStringCFB(encryptedkey, masterCred, decryptedkey);
        status = crypto::DecryptStringGCM(encryptedkey, masterCred, decryptedkey);
        if(status == ret::A_OK) {
            fi->set_file_credentials_key(decryptedkey);
            credOut = fi->file_credentials();
        }
        else {
            std::cout<< " INVALID FILEKEY, POSTFILESTRATEGY " << std::endl;
        }
    }
    return status;
}

int PostFileStrategy::ProcessFile(const std::string& requestType,
                                  const std::string& url,
                                  const std::string& filepath,
                                  const Credentials& fileCredentials,
                                  FileInfo* pFi,
                                  Response& resp) {
    int status = ret::A_OK;

    // Hash file
    //  Check if there is a previous version's metadata.
    //      compare hashes, if it is the same, don't do anything.
    //      if it is different, begin chunking process
    //          if the file is new, just linearly chunk
    //          if the file is versioned 
    //              compare each chunk 
    //              only upload new chunks 
    //              reference old chunks


    std::cout<< "processing file ... " <<std::endl;
    std::cout<< "filepath : " << filepath << std::endl;
    std::cout<< "request type : " << requestType << std::endl;
    std::cout<< "url : " << url << std::endl;

    ChunkPost old_post;
    Parent parent;
    if(requestType == "PUT") {
        std::cout<<" GETTING OLD POST : " << std::endl;
        Response response;
        netlib::HttpGet(url, NULL, &access_token_, response);
        if(response.code == 200) {
            jsn::DeserializeObject(&old_post, response.body);
        }

        std::cout<<" CODE :" << response.code << std::endl;
        // extract parents

        parent.version = old_post.version()->id;

    }

    std::string protocol, host, path;
    netlib::ExtractHostAndPath(url, protocol, host, path);
            
    boost::asio::io_service io_service; 
    Connection socket(&io_service);
    socket.Initialize(url);
    // spin off thread to hash file
            
    if(status == ret::A_OK) {
        std::string boundary;
        utils::GenerateRandomString(boundary, 20);
        std::string fileKey = fileCredentials.key();

        // Build request
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        netlib::BuildRequestHeader(requestType, url, boundary, &access_token_, request_stream); 
        
        // Start the request
        socket.Write(request);
        //socket.Write(chunkedBody);

        const unsigned int filesize = utils::CheckFilesize(filepath);
        // start the process
        std::cout<<" attempting to open filepath : " << filepath << std::endl;
        std::cout<<" file size : " << filesize << std::endl;

        std::ifstream ifs;
        ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

        unsigned int count = 0;
        unsigned int totalread = 0; // total read count
        if (ifs.is_open()) {
            std::vector<std::string> chunk_list;
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
                            std::string chunk_name;
                            ProcessChunk(chunk, fileKey, boundary, chunkcount, socket, pFi, chunk_name);
                            chunk_list.push_back(chunk_name);
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
                        std::string chunk_name;
                        ProcessChunk(chunk, fileKey, boundary, chunkcount, socket, pFi, chunk_name);
                        chunk_list.push_back(chunk_name);
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
                    std::string chunk_name;
                    ProcessChunk(remainder, fileKey, boundary, chunkcount, socket, pFi, chunk_name);
                    chunk_list.push_back(chunk_name);
                    chunkcount++;
                    break;
                }
            }

            // Send body
            std::cout<<" SENDING BODY AFTER CHUNKS " << std::endl; 
            std::cout<<" CHUNKS? : " << pFi->GetChunkInfoList()->size() << std::endl;
                    // Build Body Form header
            ChunkPost p;
            if(requestType == "PUT")
                p.PushBackParent(parent);
            PrepareChunkPost(chunk_list, old_post, pFi, p);
            std::string body; // we send an empty body for now
            jsn::SerializeObject(&p, body);

            std::cout<<" OUTGOING : " << body << std::endl;

            boost::asio::streambuf requestBody;
            std::ostream part_stream(&requestBody);
            netlib::BuildBodyForm(p.type(), body, boundary, part_stream);
            netlib::AddEndBoundry(part_stream, boundary);

            // Chunk the Body 
            boost::asio::streambuf chunkedBody;
            std::ostream partbuf(&chunkedBody);

            netlib::ChunkEnd(requestBody, partbuf);
            socket.Write(chunkedBody);

            std::cout<<" interpreting response " << std::endl;
            socket.InterpretResponse(resp);
        }
        else {
            status = ret::A_FAIL_OPEN_FILE;
        }
    }

    std::cout<<" code : " << resp.code << std::endl;
    std::cout<<" body : " << resp.body << std::endl;

    if(resp.code != 200) { 
        log::LogHttpResponse("MAN#12409", resp);
        status = ret::A_FAIL_NON_200;
    }



    return status;
}

int PostFileStrategy::PrepareChunkPost(std::vector<std::string>& chunk_list, 
                                       ChunkPost& prev_post,
                                       FileInfo* fi, 
                                       ChunkPost& out) {
    int status = ret::A_OK;

    FileInfo::ChunkMap cm;
    std::cout<<" CURRENT CHUNK COUNT : " << fi->GetChunkInfoList()->size() << std::endl;
    for(unsigned int i=0; i<chunk_list.size(); i++) {
        std::cout<<" checking : " << chunk_list[i] << std::endl;
        // Run through chunks
        if(fi->DoesChunkExist(chunk_list[i])) {
            std::cout<<" EXISTS BRO " << std::endl;
            //  - determine which ones still exist update them
            //      - update order numbers
            //  - remove old ones
            ChunkInfo ci = *(fi->GetChunkInfo(chunk_list[i]));
            ci.set_position(i);
            cm[chunk_list[i]] = ci;
            std::cout<<" CI name : " << chunk_list[i] << std::endl;
        }
    }

    fi->set_chunks(cm);
    out.set_chunk_info_list(cm);

    // Copy over previous attachments that are still relevant
    Post::AttachmentMap* pvec = prev_post.attachments();
    Post::AttachmentMap::iterator itr = prev_post.attachments()->begin();
    if(pvec) {
        for(;itr != prev_post.attachments()->end(); itr++) {
            if(fi->DoesChunkExist(itr->second.name)) {
                // copy attachemnt
                out.PushBackAttachment(itr->second);
                std::cout<<" PUSHED BACK ATTACHMENT " << std::endl;
            }
        }
    }

    std::cout<<" NEW CHUNK COUNT : " << fi->GetChunkInfoList()->size();
    return status;
}

int PostFileStrategy::ProcessChunk(const std::string& chunk, 
                                   const std::string& file_key, 
                                   const std::string& request_boundary,
                                   const unsigned int chunk_count,
                                   Connection& socket,
                                   FileInfo* fi,
                                   std::string& name_out) {
    int status = ret::A_OK;

    std::string finished_chunk, chunk_name;
    ChunkInfo ci;
    TransformChunk(chunk, 
                   file_key, 
                   finished_chunk, 
                   chunk_name, 
                   ci);

    if(fi && !fi->DoesChunkExist(chunk_name)) {
        std::cout<<" SENDING " << std::endl;
        std::cout<<" CHUNK NAME : " << chunk_name << std::endl;
        //FileInfo::ChunkMap* pList = fi->GetChunkInfoList();
        // Push chunk back into fileinfo
        std::cout<<" PUSHING BACK " << std::endl;
        fi->PushChunkBack(ci);
        std::cout<<" SIZE NOW : " << fi->GetChunkInfoList()->size() << std::endl;

        SendChunk(finished_chunk, 
                  chunk_name, 
                  request_boundary, 
                  socket, 
                  chunk_count, 
                  false);
    }

    name_out = chunk_name;

    return status;
}

int PostFileStrategy::SendChunk(const std::string& chunk, 
                                const std::string& chunk_name,
                                const std::string& boundary,
                                Connection& socket,
                                //boost::asio::ssl::stream<tcp::socket&>& ssl_sock,
                                const unsigned int count,
                                bool end) {
    int status = ret::A_OK;
    // Build Attachment
    boost::asio::streambuf attachment;
    std::ostream attachmentstream(&attachment);
    netlib::BuildAttachmentForm(chunk_name, chunk, boundary, count, attachmentstream);

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
        status = WriteToSocket(socket, partEnd);
    }
    else {
        // carry on
        // Chunk the part
        boost::asio::streambuf part;
        std::ostream chunkpartbuf(&part);
        netlib::ChunkPart(attachment, chunkpartbuf);
        status = WriteToSocket(socket, part);
    }


    return status;
}
int PostFileStrategy::WriteToSocket(Connection& socket,
                                    boost::asio::streambuf& buffer) {
    int breakcount = 0;
    int retrycount = 20;

    int status = ret::A_OK;
    unsigned int buffersize = buffer.size();
    for(unsigned int i=0; i < retrycount; i++) {
        try {
            status = ret::A_OK;
            boost::timer::cpu_timer::cpu_timer t;
            //size_t byteswritten = boost::asio::write(ssl_sock, buffer, errorcode); 
            size_t byteswritten = socket.Write(buffer);
            std::cout<<" bytes written : " << byteswritten << std::endl;
            boost::timer::cpu_times time = t.elapsed();
            boost::timer::nanosecond_type elapsed = (time.system + time.user);

            // To seconds
            elapsed *= 0.0000000001; 
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
        catch(boost::system::system_error& err) {
            std::cout<<" Write Socket error : " << err.what() << std::endl;
            status = ret::A_FAIL_SOCKET_WRITE;
        }
    }

    return status;
}


int PostFileStrategy::TransformChunk(const std::string& chunk, 
                                     const std::string& fileKey,
                                     std::string& finalizedOut, 
                                     std::string& nameOut, 
                                     ChunkInfo& out) {
    int status = ret::A_OK;
    Credentials chunkCred;
    chunkCred.set_key(fileKey);

    // Calculate plaintext hash
    std::string plaintextHash;
    crypto::GenerateHash(chunk, plaintextHash);

    std::cout<<" PLAINTEXT : " << plaintextHash << std::endl;

    // create chunk name (hex encoded plaintext hash)
    std::string chunkName;
    utils::StringToHex(plaintextHash, chunkName);

    // Compress
    std::string compressedChunk;
    compress::CompressString(chunk, compressedChunk);

    std::string compressedHash;
    crypto::GenerateHash(compressedChunk, compressedHash);
    std::cout<<" COMPRESSED HASH : " << compressedHash << std::endl;

    // Encrypt
    std::string encryptedChunk;
    std::string iv;
    crypto::GenerateIv(iv);
    chunkCred.set_iv(iv);

    std::cout<<" FILE KEY : " << fileKey << std::endl;
    std::cout<< " IV : " << iv << std::endl;
    //crypto::EncryptStringCFB(compressedChunk, chunkCred, encryptedChunk);
    crypto::EncryptStringGCM(compressedChunk, chunkCred, encryptedChunk);

    // Base64 Encode
    std::string finishedChunk;
    crypto::Base64EncodeString(encryptedChunk, finishedChunk);
    finalizedOut = finishedChunk;

    std::string ciphertextHash;
    crypto::GenerateHash(encryptedChunk, ciphertextHash);

    std::cout<<" CIPHER TEXT : " << ciphertextHash << std::endl;

    // Fill Out Chunk info object
    out.set_chunk_name(chunkName);
    out.set_plaintext_mac(plaintextHash);
    out.set_ciphertext_mac(ciphertextHash);
    out.set_iv(iv);

    nameOut = chunkName;

    return status;
}

void PostFileStrategy::UpdateFileInfo(const Credentials& fileCred, 
                                      const std::string& filepath, 
                                      const std::string& chunkpostid,
                                      const std::string& post_version,
                                      FileInfo* fi) {
    std::string filename;
    utils::ExtractFileName(filepath, filename);
    unsigned int filesize = utils::CheckFilesize(filepath);

    // Encrypt File Key
    MasterKey mKey;
    credentials_manager_->GetMasterKeyCopy(mKey);

    std::string mk;
    mKey.GetMasterKey(mk);

    fi->set_post_version(post_version);

    // Insert File Data
    fi->set_chunk_post_id(chunkpostid);
    fi->set_filepath(filepath);
    fi->set_filename(filename);
    fi->set_file_size(filesize);
    fi->set_file_credentials(fileCred);

    // Encrypt File Key
    std::string fileKey = fileCred.key();
    std::string fileIv = fileCred.iv();

    std::cout<<" Update file info file key : " << fileKey << std::endl;
    std::cout<<" Update file info file iv : " << fileIv << std::endl;

    Credentials fCred;
    fCred.set_key(mk);
    fCred.set_iv(fileIv);

    std::string encryptedKey;
    //crypto::EncryptStringCFB(fileKey, fCred, encryptedKey);
    crypto::EncryptStringGCM(fileKey, fCred, encryptedKey);

    fi->set_file_credentials_iv(fileIv);
    fi->set_file_credentials_key(fileKey);
    fi->set_encrypted_key(encryptedKey);

    // Insert file info to manifest
    file_manager_->InsertToManifest(fi);
}

FileInfo* PostFileStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = file_manager_->GetFileInfo(filepath);
    if(!fi)
        fi = file_manager_->CreateFileInfo();
    return fi;
}

}//namespace
