#ifndef FILETABLE_H_
#define FILETABLE_H_
#pragma once

#include <string>
#include "constants.h"
#include "tablehandler.h"
#include "fileinfo.h"

namespace attic {

class FileTable : public TableHandler {
    friend class Manifest;
    FileTable(sqlite3* db) : TableHandler(db, cnst::g_filetable) {}
    FileTable(const FileTable& rhs) : TableHandler(rhs.db(), rhs.table_name()) {}
    FileTable operator=(const FileTable& rhs) { return *this; }
public:
    ~FileTable() {TableHandler::~TableHandler();}

    bool CreateTable();
    bool IsFileInManifest(const std::string &filepath);
    bool InsertFileInfo(const FileInfo& fi);
};

} //namespace
#endif

