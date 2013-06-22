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
}

FileSync::~FileSync() {
    file_manager_ = NULL;
}

void FileSync::Initialize() {
    if(!thread_) {
        running_ = true;
        std::cout<<" starting worker thread ... " << std::endl;
        thread_ = new boost::thread(&FileSync::Run, this);
    }
}

void FileSync::Shutdown() {
    std::cout<<" exiting file sync " << std::endl;
    if(thread_) {
        set_running(false);
        std::cout<<" exiting worker thread .. " << std::endl;
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
    std::cout<<" exiting file sync " << std::endl;
}

void FileSync::Run() {
    bool val = false;
    while(running()) {
        FilePost fp;

        pq_mtx_.Lock();
        unsigned int size = post_queue_.size();
        if(size > 0) {
            fp = post_queue_.front();
            post_queue_.pop_front();
            size--;
            val = true;
        }
        pq_mtx_.Unlock();

        if(val) {
            ProcessFilePost(fp); 
            val = false;
        }
        else {
            sleep::mil(100);
        }
    }
}

void FileSync::PushBack(const FilePost& p) {
    pq_mtx_.Lock();
    post_queue_.push_back(p);
    pq_mtx_.Unlock();
}

int FileSync::ProcessFilePost(FilePost& p) {
    int status = ret::A_OK;
    FileManager* fm = file_manager_;
    if(!fm) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;

    FileHandler fh(file_manager_);
    FileInfo fi;
    if(!master_key_.empty()) { 
        std::cout<<" deserializing into file info " << std::endl;
        fh.DeserializeIntoFileInfo(p, master_key_, fi);
        std::cout<<" name : " << fi.filename() << std::endl;
        std::cout<<" path : " << fi.filepath() << std::endl;
    }
    else {
        std::cout<<" failed to get master key " << std::endl;
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
                std::cout<<" comparing hashes : " << std::endl;
                std::cout<<"\t local hash : " << local_fi.plaintext_hash() << std::endl;
                std::cout<<"\t incoming hash : " << fi.plaintext_hash() << std::endl;
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
    return status;
}
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

