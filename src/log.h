#ifndef LOG_H_
#define LOG_H_
#pragma once

#include <sstream>
#include <string>
#include <stdio.h>
#include <fstream>

#include "mutexclass.h"

class SyncBuffer : MutexClass
{
    SyncBuffer();
    SyncBuffer(const SyncBuffer& rhs) {}
    SyncBuffer operator=(const SyncBuffer& rhs) { return *this; }
public:
    ~SyncBuffer();

    void Startup(const std::string& directory);
    void Shutdown();

    static SyncBuffer* GetInstance();

    void PushToBuffer(std::ostringstream& os);

private:
    static SyncBuffer* m_pInstance;

    std::string m_Filepath;
    std::ofstream m_Ofs;
};

class Logger
{
public:
    enum LogLevel
    {
        UNKNOWN=0,
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        DEBUG1
    };
private:

    static std::string ToString(Logger::LogLevel level);
    Logger(const Logger& rhs) {}
    Logger operator=(const Logger& rhs) { return *this; }
public:
    Logger();
    ~Logger();

    void Log(Logger::LogLevel level, const std::string& input);
    void PrintBuffer();

private:
    SyncBuffer* m_pSyncBuffer;
    std::ostringstream os;
};

namespace log
{
    static void InitializeLogging(const std::string& directory)
    {
        SyncBuffer::GetInstance()->Startup(directory);
    }

    static void ShutdownLogging()
    {
        SyncBuffer::GetInstance()->Shutdown();
    }
}

// Definatley will not work on windows, jsut cperf time for a precision timer 
// windows.h
#include <sys/time.h>
inline std::string TimeNow()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
    return result;
}

inline std::string TimeNowNoFormat()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    sprintf(result, "%s%03ld", buffer, (long)tv.tv_usec / 1000); 

    std::string buf(result);
    size_t pos = buf.find(":");
    while(pos != (size_t)-1)
    {
        buf.erase(pos, 1);
        pos = buf.find(":");
    }

    return buf;

}

#endif

