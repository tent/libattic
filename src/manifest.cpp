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
    config_table_ = NULL;
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
    if(config_table_) {
        delete config_table_;
        config_table_ = NULL;
    }
    if(shared_post_table_) {
        delete shared_post_table_;
        shared_post_table_ = NULL;
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
    if(!db_) return false;
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
    if(!config_table_) {
        config_table_ = new ConfigTable(db_);
        if(!config_table_->CreateTable())
            return false;
    }
    if(!shared_post_table_) {
        shared_post_table_ = new SharedPostTable(db_);
        if(!shared_post_table_->CreateTable())
            return false;
    }
    return true;
}

}//namespace

