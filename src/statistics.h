#ifndef STATISTICS_H_
#define STATISTICS_H_
#pragma once

#include <map>
#include <string>
#include "mutexclass.h"

namespace attic {

class Statistics : public MutexClass {
    Statistics(){}
    ~Statistics(){}
    Statistics(const Statistics& rhs) {}
    Statistics operator=(const Statistics& rhs) { return *this; }
public:
    void Shutdown();
    static Statistics* instance();

    void IncrementValue(const std::string& key);
    void DecrementValue(const std::string& key);
    void PrintStats();

private:
    typedef std::map<std::string, unsigned int> StatsMap;
    StatsMap stats_map_;

    static Statistics* instance_;
};

} //namespace
#endif

