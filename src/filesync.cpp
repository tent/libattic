#include "filesync.h"

#include "event.h"
#include "filesystem.h"
#include "logutils.h"
#include "renamehandler.h"
#include "filehandler.h"

namespace attic {

FileSync::FileSync(FileManager* fm,
                   const AccessToken at,
                   const std::string& entity_url,
                   const std::string& posts_feed,
                   const std::string& post_path,
                   const std::string& master_key) {
    running_ = false;
    at_ = at;
    file_manager_ = fm;

    // avoid cow
    posts_feed_.append(posts_feed.c_str(), posts_feed.size());
    post_path_.append(post_path.c_str(), post_path.size());
    entity_url_.append(entity_url.c_str(), entity_url.size());
    master_key_.append(master_key.c_str(), master_key.size());

    utils::FindAndReplace(post_path, "{entity}", entity_url_, post_path_);
    thread_ = NULL;
}

FileSync::~FileSync() {
    file_manager_ = NULL;
}

void FileSync::Initialize() {
    if(!thread_) {
        set_running(true);
        std::cout<<" starting file sync thread ... " << std::endl;
        thread_ = new boost::thread(&FileSync::Run, this);
    }
}

void FileSync::Shutdown() {
    std::cout<<" exiting file sync thread " << std::endl;
    if(thread_) {
        set_running(false);
        std::cout<<" exiting file sync thread " << std::endl;
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
    std::cout<<" exiting file sync thread " << std::endl;
}

void FileSync::Run() {
    std::cout<<" FileSync Running " << std::endl;
    bool val = false;
    while(running()) {
        FilePost fp;

        pq_mtx_.Lock();
        unsigned int size = post_queue_.size();
        if(size > 0) {
            std::cout<<" Retrieving FilePost " << std::endl;
            fp = post_queue_.front();
            post_queue_.pop_front();
            size--;
            val = true;
        }
        pq_mtx_.Unlock();

        if(val) {
            std::cout<<" Processing File Post "<< fp.id() << std::endl;
            ProcessFilePost(fp); 
            std::cout<<"Done Processing File Post "<< fp.id() << std::endl;
            val = false;
        }
        else {
            sleep::mil(100);
        }
    }
    std::cout << " FileSync Exiting " << std::endl;
}

void FileSync::PushBack(const FilePost& p) {
    pq_mtx_.Lock();
    post_queue_.push_back(p);
    pq_mtx_.Unlock();
}

int FileSync::ProcessFilePost(FilePost& p) {
    int status = ret::A_OK;
    // check if file exists, in manifest and on disk
    // - if yes, 
    //      - compare filenames, local and in post
    //      - compare parent_posts
    //          - if differ, update and move file to new path
    //      - compare hashes
    //          - if different initiate a download
    // - else
    //      - verify folder exists
    //          - insert into manifest
    //          - initiate a download

    std::ostringstream plog; 
    plog << "*****************************************************" << std::endl;
    plog << "ProcessFilePost : " << p.id() << std::endl;
    FileInfo fi;
    if(ExtractFileInfo(p, fi)) {
        plog << " filename (post) : " << fi.filename() << std::endl;
        plog << " filepath (post) : " << fi.filepath() << std::endl;
        plog << " folder_post (post) : " << fi.folder_post_id() << std::endl;
        bool pull = false;
        FileHandler fh(file_manager_);
        if(file_manager_->DoesFileExistWithPostId(p.id())) {
            FileInfo local_fi;
            file_manager_->GetFileInfoByPostId(p.id(), local_fi);
            plog << " filename (cache) : " << local_fi.filename() << std::endl;
            plog << " filepath (cache) : " << local_fi.filepath() << std::endl;
            plog << " folder_post (cache) : " << local_fi.folder_post_id() << std::endl;
            if(fi.filename() != local_fi.filename()) {
                plog << " rename " << std::endl;
                // rename
                RenameHandler rh(file_manager_);
                rh.RenameFileLocalCache(p.id(), fi.filename());
            }
            if(fi.folder_post_id() != local_fi.folder_post_id()){
                plog << " move to new folder " << std::endl;
                // move to new folder
                fh.UpdateFilepath(p.id(), fi.folder_post_id());
            }
            if(fi.plaintext_hash() != local_fi.plaintext_hash()) {
                plog << " init dl " << std::endl;
                // init download
                pull = true;
            }
        }
        else {
            // Doesn't exist in the manifest
            // Insert into manifest
            if(fi.file_credentials_iv() == p.iv_data()) {
                plog <<" inserting into manifest " << std::endl;
                file_manager_->InsertToManifest(&fi);
            }
            else { 
                std::ostringstream err;
                err<<" INVALID IV DATA : " << std::endl;
                err<<" filename : " << fi.filepath() << std::endl;
                err<<" file info : " << fi.file_credentials_iv() << std::endl;
                err<<" file post : " << p.iv_data() << std::endl;
                log::LogString("149128591245", err.str());
            }
            pull = true;
        }
        if(pull) RaisePullRequest(p, fi);
    }
    else {
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }

    plog << "ProcessFilePost status : " << status << std::endl;
    plog << "*****************************************************" << std::endl;
    std::cout<< plog.str() << std::endl;
    return status;
}

bool FileSync::ExtractFileInfo(FilePost& p, FileInfo& out) {
    bool ret = false;
    FileHandler fh(file_manager_);
    if(!master_key_.empty()) { 
        fh.DeserializeIntoFileInfo(p, master_key_, out);
        ret = true;
    }
    else {
        std::ostringstream err;
        err <<" Empty master key " << std::endl;
        log::LogString("fsync_124801", err.str());
    }
    return ret;
}
/*
int FileSync::ProcessFilePost(FilePost& p) {
    int status = ret::A_OK;
    std::ostringstream plog;
    plog << "**************************************************" << std::endl;
    plog << "Process File Posts "  << std::endl;

    FileManager* fm = file_manager_;
    if(!fm) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;

    FileHandler fh(file_manager_);
    FileInfo fi;
    if(!master_key_.empty()) { 
        plog <<" deserializing into file info " << std::endl;
        fh.DeserializeIntoFileInfo(p, master_key_, fi);
        plog <<" name : " << fi.filename() << std::endl;
        plog <<" path : " << fi.filepath() << std::endl;
    }
    else {
        plog <<" failed to get master key " << std::endl;
        return ret::A_FAIL_INVALID_MASTERKEY;
    }

    // Check if any aliases exist, and fix
    RenameHandler rh(file_manager_);
    if(!rh.CheckForRename(fi, p.id())) {
        // Get Local file info
        std::string filepath = fi.filepath();

        bool bPull = false;
        FileInfo local_fi;
        if(fm->GetFileInfo(filepath, local_fi)) {
            std::string canonical_path;
            fm->GetCanonicalPath(filepath, canonical_path);
            // check if file exists, locally
            if(local_fi.deleted()) {
                bPull = false;
            }
            else if(!fs::CheckFilepathExists(canonical_path)) {
                fm->InsertToManifest(&fi); // Update local cache
                bPull = true;
            }
            else if(fs::CheckFilepathExists(canonical_path)) {
                // compare hashes
                plog <<" comparing hashes : " << std::endl;
                plog <<"\t local hash : " << local_fi.plaintext_hash() << std::endl;
                plog <<"\t incoming hash : " << fi.plaintext_hash() << std::endl;
                if(local_fi.plaintext_hash() != fi.plaintext_hash()) {
                    fm->InsertToManifest(&fi); // Update local cache
                    bPull = true;
                }
            }
            if(local_fi.post_version() != p.version().id()) { 
                fm->InsertToManifest(&fi); // Update local cache
                bPull = true;
            }
        }
        else {
            std::cout<<" NOT IN MANIFEST PULL " << std::endl;
            // Insert into manifest
            if(fi.file_credentials_iv() == p.iv_data()) {
                std::cout<<" inserting into manifest " << std::endl;
                fm->InsertToManifest(&fi);
            }
            else { 
                std::ostringstream err;
                err<<" INVALID IV DATA : " << std::endl;
                err<<" filename : " << fi.filepath() << std::endl;
                err<<" file info : " << fi.file_credentials_iv() << std::endl;
                err<<" file post : " << p.iv_data() << std::endl;
                log::LogString("149128591245", err.str());
            }
            // Doesn't exist in the manifest
            bPull = true;
        }
        if(bPull) RaisePullRequest(p, fi);
    }

    plog << " ProcessFilePost status : " << status << std::endl;
    plog << "**************************************************" << std::endl;
    std::cout<< plog.str() << std::endl;
    return status;
}
*/

int FileSync::RaisePullRequest(const FilePost& p, FileInfo& fi) {
    int status = ret::A_OK;

    std::string filepath = fi.filepath();
    if(!filepath.empty()) { 
        event::RaiseEvent(event::Event::REQUEST_PULL, filepath, NULL);
    }
    else {
        status = ret::A_FAIL_INVALID_FILEPATH;
    }
    return status;
}

bool FileSync::running() {
    bool t;
    r_mtx_.Lock();
    t = running_;
    r_mtx_.Unlock();
    return t;
}

void FileSync::set_running(bool r) {
    r_mtx_.Lock();
    running_ = r;
    r_mtx_.Unlock();
}

} //namespace

