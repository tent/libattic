#include "scandirectorytask.h"

#include "configmanager.h"
#include "filesystem.h"
#include "event.h"
#include "sleep.h"

namespace attic { 

ScanDirectoryTask::ScanDirectoryTask(FileManager* fm,
                                     void (*callback)(int, char**, int, int))
                                     :
                                     ManifestTask(Task::SCANDIRECTORY,
                                                  fm,
                                                  callback) {
}

ScanDirectoryTask::~ScanDirectoryTask() {}

void ScanDirectoryTask::OnStart() {} 
void ScanDirectoryTask::OnPaused() {} 
void ScanDirectoryTask::OnFinished() {}

void ScanDirectoryTask::RunTask() {
    std::string working_dir;
    ConfigManager::GetInstance()->GetValue("working_dir", working_dir);
    if(!working_dir.empty()){
        // Scan folder
        FileVector file_list;
        fs::ScanDirectory(working_dir, file_list);

        CompareToLocalCash(file_list);
        // compare what is there to what is in the manifest
        //  - looking for untracked files
        //      - run request push task on the filepath
    }

    SetFinishedState();
}

void ScanDirectoryTask::CompareToLocalCash(const FileVector& file_list) {
    // Check if local file exists in the manifest
    FileVector::const_iterator itr = file_list.begin();
    for(;itr != file_list.end(); itr++) {
        std::string filepath = *itr;
        if(!(file_manager()->DoesFileExist(filepath))){
            // Push task
            fs::GetCanonicalPath((*itr), filepath);
            event::RaiseEvent(attic::event::Event::REQUEST_PUSH, filepath, NULL);
            sleep::sleep_milliseconds(100); // back off the file manager, this isn't super critical
        }
    }
}

}//namespace
