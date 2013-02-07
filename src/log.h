#ifndef LOG_H_
#define LOG_H_
#pragma once

#include <sstream>
#include <string>
#include <stdio.h>
#include <fstream>

#include "mutexclass.h"

#include <iostream>

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

    void GetFilepath(std::string& out) 
    { 
        std::cout<<" here" << std::endl;
        Lock();
        out = m_Filepath;
        Unlock();
    }

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

    void Log(const Logger::LogLevel level, const std::string& input);
    void GetLogFilepath(std::string& out)
    {
        std::cout<<"232"<<std::endl;
        if(m_pSyncBuffer)
        {
            std::cout<<"232"<<std::endl;
            m_pSyncBuffer->GetFilepath(out);
            std::cout<<"232"<<std::endl;
        }

    }

    void PrintBuffer();

private:
    SyncBuffer* m_pSyncBuffer;
    std::ostringstream os;
};

namespace alog
{
    static void InitializeLogging(const std::string& directory)
    {
        SyncBuffer::GetInstance()->Startup(directory);
    }

    static void ShutdownLogging()
    {
        SyncBuffer::GetInstance()->Shutdown();
    }

    static void Log(const Logger::LogLevel level, const std::string& input) // One off
    {

        Logger logger;
        logger.Log(level, input);
    }

    static void GetCurrentLogFilepath(std::string& out)
    {
        std::cout<<" CUrrent logs?" << std::endl;
        Logger logger;

        std::cout<<" CUrrent logs?" << std::endl;
        logger.GetLogFilepath(out);

        std::cout<<" CUrrent logs?" << std::endl;

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

#include <ctime>
#include <string.h>
inline std::string GetDate()
{
    std::string date;
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    char buf[256] = {'\0'};
    sprintf(buf, "%d", (now->tm_year + 1900));
    date += buf;

    date += '-';
    memset(buf, '\0', 256);
    sprintf(buf, "%d", (now->tm_mon + 1));
    date += buf;
    
    date += '-';
    memset(buf, '\0', 256);
    sprintf(buf, "%d", (now->tm_mday));
    date += buf;

    return date;
}




#endif

