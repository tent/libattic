#ifndef CHUNKTABLE_H_
#define CHUNKTABLE_H_
#pragma once

#include <string>
#include "constants.h"
#include "tablehandler.h"

namespace attic {

class ChunkTable : public TableHandler {
    friend class Manifest;

    ChunkTable(sqlite3* db) : TableHandler(db, cnst::g_chunktable) {}
    ChunkTable(const ChunkTable& rhs) : TableHandler(rhs.db(), rhs.table_name()) {}
    ChunkTable operator=(const ChunkTable& rhs) { return *this; }
public:
    ~ChunkTable() {}

    bool CreateTable();

    bool InsertChunkDigest(const std::string& digest, const std::string& plaintext_mac);
    bool IsDigestInManifest(const std::string& digest);
    bool IsPlaintextMacInManifest(const std::string& plaintext_mac);

    bool GetChunkDigest(const std::string& plaintext_mac, std::string& out);
    bool GetPlaintextMac(const std::string& digest, std::string& out);

};


} // namespace

#endif

