#include "tablehandler.h"

namespace attic { 

TableHandler::TableHandler(sqlite3* db, const std::string& name) {
    db_ = db;
    stmt_ = NULL;
    table_name_ = name;
}

TableHandler::~TableHandler() {
    db_ = NULL;
    stmt_ = NULL;
}

bool TableHandler::Exec(const std::string& query, std::string& error_out) const {
    if(!db_ || query.empty())
        return false;

    char* szError = NULL;
    int rc = sqlite3_exec(db_, 
                          query.c_str(), 
                          NULL, 
                          NULL,
                          &szError);

    if(rc != SQLITE_OK) {
        if(szError) {
            error_out.append(szError);
            sqlite3_free(szError);
            szError = NULL;
        }
        return false;
    }
    return true;
}

bool TableHandler::Select(const std::string& select, SelectResult& out, std::string& error_out) const {
    if(!db_ || select.empty())
        return false;

    char *szError = NULL;
    int rc = sqlite3_get_table(db_,
                               select.c_str(),     /* SQL statement to be evaluated */
                               &out.results_,      /* Results of the query */
                               &out.row_,          /* Number of result rows written here */
                               &out.col_,          /* Number of result columns written here */
                               &szError);          /* Error msg written here, get table method handles cleanup */
    if(rc != SQLITE_OK) {
        if(szError)
            error_out.append(szError);
        return false;
    }

    return true;
}

bool TableHandler::PrepareStatement(const std::string& statement, std::string& error_out) {
    sqlite3_stmt* stmt = NULL;
    int ret = sqlite3_prepare_v2(db_, statement.c_str(), -1, &stmt, 0);

    if(ret != SQLITE_OK) {
        error_out = "Failed to prepare statement";
        stmt = NULL;
        return false;
    }
    stmt_ = stmt;
    return true;
}

bool TableHandler::StepStatement(std::string& error_out) {
    if(stmt_) {
        int ret = sqlite3_step(stmt_);
        if(ret != SQLITE_DONE) {
            error_out += "Step, Error Message : ";
            error_out += sqlite3_errmsg(db_);
            error_out += "\n";
        }
        else  {
            return true;
        }
    }
    return false;
}

bool TableHandler::FinalizeStatement(std::string& error_out) {
    if(stmt_) {
        int ret = sqlite3_finalize(stmt_);
        if(ret != SQLITE_OK) {
            error_out += "Finalize Error Message : ";
            error_out +=  sqlite3_errmsg(db_);
            error_out += "\n";
        }
        else {
            return true;
        }
    }
    return false;
}

bool TableHandler::BindInt(unsigned int position, int value, std::string& error_out) {
    if(stmt_) {
        int ret = sqlite3_bind_int(stmt_, position, value);
        if(ret != SQLITE_OK) {
            error_out += "Bind Int Error, position : ";
            error_out += position;
            error_out += " Error Message : ";
            error_out +=  sqlite3_errmsg(db_);
            error_out += "\n";
        }
        else {
            return true;
        }
    }
    return false;
}

bool TableHandler::BindText(unsigned int position, const std::string& value, std::string& error_out) {
    if(stmt_) {
        int ret = sqlite3_bind_text(stmt_, position, value.c_str(), value.size(), SQLITE_STATIC);
        if(ret != SQLITE_OK) {
            error_out += "Bind Text Error, position : ";
            error_out += position;
            error_out += " Error message: ";
            error_out +=  sqlite3_errmsg(db_);
            error_out += "\n";
        }
        else {
            return true;
        }
    }
    return false;
}

bool TableHandler::BindBlob(unsigned int position, const std::string& value, std::string& error_out) {
    if(stmt_) {
        int ret = sqlite3_bind_blob(stmt_, position, value.c_str(), value.size(), SQLITE_TRANSIENT);
        if(ret != SQLITE_OK) {
            error_out += "Bind Blob Error, position : ";
            error_out += position;
            error_out += " Error message: ";
            error_out +=  sqlite3_errmsg(db_);
            error_out += "\n";
        }
        else {
            return true;
        }
    }
    return false;
}

bool TableHandler::ClearStatement() {
    // TODO :: probably some other step with clearing a non finalized statement, look it up at some point
    stmt_ = NULL;
}


} // namespace

