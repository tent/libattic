#ifndef TABLEHANDLER_H_
#define TABLEHANDLER_H_
#pragma once

#include <string>
#include <sqlite3.h>

namespace attic {

class SelectResult { 
    friend class Manifest;
    friend class TableHandler;
public:
    SelectResult() {
        row_ = 0;
        col_ = 0;
    }
    ~SelectResult() {
        sqlite3_free_table(results_);
    }

    std::string operator[](const unsigned int n) { return results_[n]; }

    int row() const { return row_; }
    int col() const { return col_; }
private:
    char** results_;
    int row_;
    int col_;
};

class TableHandler {
public:
    TableHandler(sqlite3* db, const std::string& name);
    virtual ~TableHandler();
    virtual bool CreateTable() = 0;
    bool Exec(const std::string& query, std::string& error_out) const;
    bool Select(const std::string& select, SelectResult& out, std::string& error_out) const;

    const std::string& table_name() const { return table_name_; }

    bool PrepareStatement(const std::string& statement, std::string& error_out);
    bool StepStatement(std::string& error_out);
    bool FinalizeStatement(std::string& error_out);
    bool BindInt(unsigned int position, int value, std::string& error_out);
    bool BindText(unsigned int position, const std::string& value, std::string& error_out);
    bool BindBlob(unsigned int position, const std::string& value, std::string& error_out);

    bool ClearStatement();
private:
    sqlite3* db_;
    sqlite3_stmt* stmt_;
    std::string table_name_;
};

}//namespace
#endif

