#include "manifest.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "utils.h"
#include "crypto.h"

namespace attic {

Manifest::Manifest() {
    db_ = NULL;
    file_table_ = NULL;
    folder_table_ = NULL;
}

Manifest::~Manifest() {
    if(file_table_) {
        delete file_table_;
        file_table_ = NULL;
    }
    if(folder_table_) {
        delete folder_table_;
        folder_table_ = NULL;
    }
}

void Manifest::SetDirectory(std::string &filepath)  { 
    filepath_ = filepath; 
    utils::CheckUrlAndAppendTrailingSlash(filepath_);
    filepath_ += "manifest";
}

int Manifest::Initialize() {
    return OpenSqliteDb();
}

int Manifest::Shutdown() { 
    if(file_table_) {
        delete file_table_;
        file_table_ = NULL;
    }
    if(folder_table_) {
        delete folder_table_;
        folder_table_ = NULL;
    }
    int status = CloseSqliteDb();
    return status;
}

int Manifest::OpenSqliteDb() {
    int status = ret::A_OK;

    int rc = sqlite3_open(filepath_.c_str(), &db_);
    if(rc) {
        // failed
        std::cout<< "Can't open database: " << sqlite3_errmsg(db_);
        std::cout<< " attempted to open : " << filepath_ << std::endl;
        status = ret::A_FAIL_TO_LOAD_MANIFEST;
        db_ = NULL;
    }
    else {
        if(!db_) {
            std::cout << " invlid db instance " << std::endl;
            status = ret::A_FAIL_TO_LOAD_MANIFEST;
        } 
        else { 
            CreateTables();
        }
    }

    return status;
}

int Manifest::CloseSqliteDb() {
    std::cout<<" attempting to close db ... " << std::endl;
    if(db_) {
        std::cout<<" sqlite close " << std::endl;
        int err = sqlite3_close(db_);
        if(err != SQLITE_OK)
            std::cout<<" FAILED TO CLOSE DB " << std::endl;
        db_ = 0;
    }
    return ret::A_OK;
}

bool Manifest::CreateTables() {
    if(!db_)
        return false;

    if(!file_table_) {
        file_table_ = new FileTable(db_);
        if(!file_table_->CreateTable())
            return false;
    }

    if(!folder_table_) {
        folder_table_ = new FolderTable(db_);
        if(!folder_table_->CreateTable())
            return false;
    }
    return true;
}

bool Manifest::UpdateAllFileInfoForFolder(const std::string& folderid) { 
    bool ret = false;
    FileTable::FileInfoList file_list;
    if(file_table_->QueryAllFilesForFolder(folderid, file_list)) {
        FileTable::FileInfoList::iterator itr = file_list.begin();
        for(;itr!=file_list.end();itr++) {
            std::string folder_id = (*itr).folder_post_id();
            std::string path;
            folder_table_->GetFolderPath(folder_id, path);
            // Update filepath
            utils::CheckUrlAndAppendTrailingSlash(path);
            path += (*itr).filename();
            ret = file_table_->set_filepath((*itr).filepath(), path);
        }
    }
    return ret;
}

}//namespace

