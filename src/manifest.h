#ifndef MANIFEST_H_
#define MANIFEST_H_
#pragma once

#include <fstream>
#include <deque>
#include <string>
#include <map>
#include <sqlite3.h>

#include "tablehandler.h"
#include "filetable.h"
#include "foldertable.h"
#include "configtable.h"
#include "fileinfo.h"
#include "folder.h"

namespace attic { 

class Manifest {
    int OpenSqliteDb();
    int CloseSqliteDb();
    bool CreateTables();
public:
    Manifest();
    ~Manifest();

    int Initialize();
    int Shutdown();

    FileTable* file_table() { return file_table_; }
    FolderTable* folder_table() { return folder_table_; }
    ConfigTable* config_table() { return config_table_; }

    bool CreateEmptyManifest();

    // Method using both tables
    bool UpdateAllFileInfoForFolder(const std::string& folderid);

    void SetDirectory(std::string &filepath); 
private:
    sqlite3*            db_;
    FileTable*          file_table_;
    FolderTable*        folder_table_;
    ConfigTable*        config_table_;

    // Manifest specific data
    std::string         filepath_;     // path to manifest file
};

} //namespace
#endif

