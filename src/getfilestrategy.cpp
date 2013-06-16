#include "getfilestrategy.h"

#include <sstream>
#include <iostream>
#include <boost/timer/timer.hpp>

#include "utils.h"
#include "netlib.h"
#include "compression.h"
#include "crypto.h"
#include "filesystem.h"
#include "event.h"
#include "logutils.h"
#include "connectionhandler.h"
#include "envelope.h"
#include "filehandler.h"
#include "folderhandler.h"
#include "posthandler.h"
#include "chunktransform.h"

namespace attic { 

GetFileStrategy::GetFileStrategy() {}
GetFileStrategy::~GetFileStrategy() {}

int GetFileStrategy::Execute(FileManager* pFileManager,
                             CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);

    post_path_ = GetConfigValue("post_path");
    std::string filepath = GetConfigValue("filepath");
    std::string post_attachment = GetConfigValue("post_attachment");
    std::string entity = GetConfigValue("entity");
    std::cout<<" starting get file strategy ... for :" << filepath << std::endl;

    if(!ValidMasterKey()) return ret::A_FAIL_INVALID_MASTERKEY;

    if(!filepath.empty()) { 
        FileHandler fi_hdlr(file_manager_);
        FolderHandler fl_hdlr(file_manager_);
        
        FileInfo fi;
        if(fi_hdlr.RetrieveFileInfo(filepath, fi)) {
            Folder folder;
            if(!fl_hdlr.GetFolderById(fi.folder_post_id(), folder)) {
                FolderPost fp;
                if(RetrieveFolderPost(fi.folder_post_id(), fp)) {
                    folder = fp.folder();
                }
                else {
                    std::ostringstream err;
                    err << " failed to find folder in manifest, attempt to pull post" << std::endl;
                    err << " alleged post id : " << fi.folder_post_id() << std::endl;
                    err << " for file : " ;
                    err << fi.filepath() << std::endl;
                    log::LogString("sdfjka111", err.str());
                    return ret::A_FAIL_FOLDER_NOT_IN_MANIFEST;
                }
            }
            if(folder.deleted()) {
                std::string error("folder deleted");
                error += " for file : ";
                error += fi.filepath();
                log::LogString("asdfaiikjkej", error);
                return ret::A_FAIL_PULL_DELETED_FOLDER;
            }

            if(fi.deleted()) return ret::A_FAIL_PULL_DELETED_FILE;
            // Retrieve file metadata
            FilePost meta_post;
            status = RetrieveFilePost(fi.post_id(), meta_post);
            if(status == ret::A_OK) {
                Credentials file_cred;
                // Get file credentials
                status = ExtractCredentials(meta_post, file_cred);
                if(status == ret::A_OK) {
                    if(!file_cred.key_empty()) {
                        // put file posts in order
                        std::string destination;
                        ConstructFilepath(fi, folder, destination);
                        // pull chunks
                        status = ConstructFile(fi, file_cred, destination);
                        if(status == ret::A_OK) {
                            // Retrieve associated folder entries and create local cache 
                            // entries for them
                            ValidateFolderEntries(meta_post);
                        }
                    }
                    else {
                        std::string error = "Invalid file key on get file";
                        log::LogString("MAOP8091", error);
                        status = ret::A_FAIL_INVALID_FILE_KEY;
                    }
                }
            }
        }
        else {
            std::string error("failed to find filepath : ");
            error += filepath;
            log::LogString("smmma0149", error);
            status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
        }
    }
    else {
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    }

    std::cout<<" GET FILE STRATEGY RETURN STATUS : " << status << std::endl;
    return status;
}
 
int GetFileStrategy::RetrieveFilePost(const std::string& post_id, FilePost& out) { 
    int status = ret::A_OK;
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

    // Get Metadata post
    PostHandler<FilePost> ph(access_token_);
    status = ph.Get(posturl, NULL, out);
    return status;
}

int GetFileStrategy::ExtractCredentials(FilePost& in, Credentials& out) {
    int status = ret::A_OK;
    
    std::string key, iv;
    key = in.key_data();
    iv = in.iv_data();

    std::string mk;
    GetMasterKey(mk);
    if(!mk.empty()) {
        Credentials FileKeyCred;
        FileKeyCred.set_key(mk);
        FileKeyCred.set_iv(iv);

        // Decrypt File Key
        std::string filekey;
        if(crypto::Decrypt(key, FileKeyCred, filekey)) {
            out.set_key(filekey);
            out.set_iv(iv);
        }
        else { 
            std::string b64_key, b64_iv;
            crypto::Base64EncodeString(in.key_data(), b64_key);
            crypto::Base64EncodeString(in.iv_data(), b64_iv);
            std::ostringstream err;
            err << " Failed to construct file key " << std::endl;
            err << " master key size : " << mk.size() << std::endl;
            err << " b64_iv : " << b64_iv << std::endl;
            err << " iv size : " << iv.size() << std::endl;
            err << " b64_key : " << b64_key << std::endl;
            err << " key size : " << key.size() << std::endl;
            log::LogString("815092", err.str());
        }
    }
    else {
        std::ostringstream err;
        err << " Invalid master key trying to extract credentials" << std::endl;
        log::LogString("1239120951", err.str());
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }

    return status;
}

int GetFileStrategy::ConstructFile(FileInfo& fi,
                                   const Credentials& file_cred,
                                   const std::string& destination_path) {
    typedef std::map<unsigned int, ChunkInfo> ChunkList;
    int status = ret::A_OK;

    std::string temp_path;
    FileHandler fh(file_manager_);
    fh.GetTemporaryFilepath(fi, temp_path);

    std::ofstream ofs;
    ofs.open(temp_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if(ofs.is_open()) {
        // Get all chunk credentials
        // move them into a map, key position value chunk info
        // interate through them using the digest against the attachemnts endpoint
        // construct
        FileInfo::ChunkMap* chunk_map = fi.GetChunkInfoList();
        if(chunk_map->size()) {
            std::cout<<" CHUNK MAP SIZE : " << chunk_map->size();
            ChunkList chunk_list;
            FileInfo::ChunkMap::iterator map_itr = chunk_map->begin();
            for(;map_itr!=chunk_map->end(); map_itr++) {
                std::cout<<" \tchunk name : " << map_itr->second.chunk_name() << std::endl;
                std::cout<<" \tpushing back position : " << map_itr->second.position() << std::endl;
                chunk_list[map_itr->second.position()] = map_itr->second;
            }
            std::cout<<" CHUNK LIST SIZE : " << chunk_list.size() << std::endl;
            std::string post_attachment = GetConfigValue("post_attachment");
            for(unsigned int i=0; i<chunk_list.size(); i++) {
                ChunkList::iterator itr = chunk_list.find(i);
                if(itr!= chunk_list.end()) {
                    std::cout<<" FOUND CHUNK # : " << i << std::endl;
                    // Get attachment
                    std::string attachment_path;
                    utils::FindAndReplace(post_attachment,
                                          "{digest}",
                                          itr->second.digest(),
                                          attachment_path);
                    std::cout<<" attachment path : " << attachment_path << std::endl;
                    std::string buffer;
                    status = RetrieveAttachment(attachment_path, buffer);
                    if(status == ret::A_OK) {
                        std::cout<<" Fetching chunk : ... " << itr->second.chunk_name() << std::endl;
                        std::string chunk;
                        status = TransformChunk(&itr->second, file_cred.key(), buffer, chunk);
                        if(status == ret::A_OK) {
                            // Append to file 
                            ofs.write(chunk.c_str(), chunk.size());
                        }
                        else {
                            std::cout<<" FAILED TRANSFORM " << std::endl;
                            std::cout<<" STATUS : "<< status << std::endl;
                            break;
                        }
                    }
                }
                else {
                    status = ret::A_FAIL_MISSING_CHUNK;
                    break;
                }
            }
        }
        else {
            status = ret::A_FAIL_ZERO_CHUNK_COUNT;
        }
        ofs.close();
    }

    if(status == ret::A_OK) {
        std::string path;
        status = ConstructFilepath(fi, path);
        if(status == ret::A_OK) {
            if(!destination_path.empty())
                try {
                    fs::MoveFile(temp_path, path);
                    log::LogString("MOVING TO PATH", path);
                }
                catch(std::exception &e) {
                    log::LogException("getfile_32987523", e);    
                }
        }
        else {
            std::ostringstream err;
            err <<" Failed to move file to path : " << path << std::endl;
            log::LogString("getfile_98512", err.str());
            status = ret::A_FAIL_MOVE_PATH;
        }
    }

    // delete temp file 
    if(fs::CheckFilepathExists(temp_path))
        fs::DeleteFile(temp_path);

    return status;
}

int GetFileStrategy::RetrieveAttachment(const std::string& url, std::string& outBuffer) {
    int status = ret::A_OK;

    std::cout<<" RETRIEVING ATTACHEMNT : " << url << std::endl;
    boost::timer::cpu_timer::cpu_timer t;
    Response response;
    status = netlib::HttpGetAttachment(url, NULL, &access_token_, response);
    boost::timer::cpu_times time = t.elapsed();
    boost::timer::nanosecond_type const elapsed(time.system + time.user);
    //std::cout<<" time : " << time << std::endl;
    std::cout<<" elapsed : "<< elapsed << std::endl;

    if(response.code == 200) {
        outBuffer = response.body;
    }
    else {
        std::cout<<" RETREIVE ATTACHMENT FAILED " << std::endl;
        std::cout<<" FAILED BODY : " << response.body << std::endl;
        status = ret::A_FAIL_NON_200;
    }

    if(elapsed > 0) {
        std::cout<<" elapsed : "<< elapsed << std::endl;
        std::cout<<" buffer size : "<< outBuffer.size() << std::endl;

        std::ostringstream speed;
        speed << (outBuffer.size() / elapsed) << std::endl; // bytes per nanosecod
        event::RaiseEvent(event::Event::DOWNLOAD_SPEED, speed.str(), NULL);
    }

    /*
    std::ostringstream oss;
    oss << outBuffer.size() << std::endl;
    event::RaiseEvent(event::Event::DOWNLOAD_TRANSFERRED, oss.str(), NULL);
    */
    return status;                                                                        
}

int GetFileStrategy::TransformChunk(const ChunkInfo* ci, 
                                    const std::string& filekey,
                                    const std::string& chunkBuffer, 
                                    std::string& out) {
    int status = ret::A_OK;
    if(ci) {
        ChunkTransform ct(chunkBuffer, filekey);
        if(ct.TransformIn(ci)) {
            if(ct.ciphertext_hash() == ci->ciphertext_mac() && 
               ct.plaintext_hash() == ci->plaintext_mac()) {
                    std::cout<<" ^^^---- HASHES ARE THE SAME ----^^^" << std::endl;
                    out = ct.finalized_data();
                    std::cout<<" decomp chunk : " << out.size() << std::endl;
                    std::cout<<" size of chunk : " << out.size() << std::endl;
            }
            else {
                status = ret::A_FAIL_INVALID_CHUNK_HASH;
            }
        }
    }
    else { 
        status = ret::A_FAIL_INVALID_PTR;
    }
    return status;
}

void GetFileStrategy::ValidateFolderEntries(FilePost& fp) {
    std::cout<<" Validating folder entires ... " << std::endl;
    std::deque<FolderPost> folder_list;
    RetrieveFolderPosts(fp, folder_list);
    std::cout<< " retrieved " << folder_list.size() << " folder posts " << std::endl;
    if(folder_list.size()) {
        FileHandler fh(file_manager_);
        // Validate folder entries exist
        std::deque<FolderPost>::iterator itr = folder_list.begin();
        for(;itr!= folder_list.end(); itr++) { 
            std::cout<<" updating folder entries " << std::endl;
            fh.UpdateFolderEntry((*itr));
        }
    }
}

void GetFileStrategy::RetrieveFolderPosts(FilePost& fp, std::deque<FolderPost>& out) {
    Post::MentionsList::iterator itr = fp.mentions()->begin();
    for(;itr!=fp.mentions()->end(); itr++) {
        FolderPost fp;
        if(RetrieveFolderPost((*itr).post, fp))
            out.push_back(fp);
    }
}

bool GetFileStrategy::RetrieveFolderPost(const std::string& post_id, FolderPost& out) {
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

    FolderPost fp;
    PostHandler<FolderPost> ph(access_token_);
    int status = ph.Get(posturl, NULL, fp);

    if(status == ret::A_OK) {
        if(fp.type().find(cnst::g_attic_folder_type) != std::string::npos) 
            return true;
    }
    else {
        log::LogHttpResponse("MASDKF111", ph.response());
    }
    return false;
}

int GetFileStrategy::ConstructFilepath(const FileInfo& fi, std::string& out) {
    int status = ret::A_OK;
    if(!fi.folder_post_id().empty()) {
        std::string posturl;
        utils::FindAndReplace(post_path_, "{post}", fi.folder_post_id(), posturl);

        FolderPost fp;
        PostHandler<FolderPost> ph(access_token_);
        status = ph.Get(posturl, NULL, fp);
        if(status == ret::A_OK) {
            ConstructFilepath(fi, fp.folder(), out);
            std::cout<<" constructed path : " << out << std::endl;
        }
        else {
            log::LogHttpResponse("00301--1-0-10", ph.response());
        }
    }
    else {
        std::ostringstream err;
        err << " Empty folder post id for file info : " << fi.filepath() << std::endl;
        log::LogString("ma-___", err.str());
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

void GetFileStrategy::ConstructFilepath(const FileInfo& fi, const Folder& folder, std::string& out) {
    std::string path;
    file_manager_->GetCanonicalFilepath(folder.folderpath(), path);
    utils::CheckUrlAndAppendTrailingSlash(path);
    out = path + fi.filename();
}

}//namespace
