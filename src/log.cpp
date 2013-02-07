#include "log.h"

#include <iostream>

#include "utils.h"

SyncBuffer* SyncBuffer::m_pInstance = 0;

SyncBuffer::SyncBuffer()
{
}

SyncBuffer::~SyncBuffer()
{
}

void SyncBuffer::Startup(const std::string& directory)
{
    if(!directory.empty())
    {
        std::string filepath;
        filepath = directory;
        utils::CheckUrlAndAppendTrailingSlash(filepath);
        filepath += GetDate() + "_log";

        std::string hold = filepath;
        static int namecount = 0;
        for(;;)
        {
            if(utils::CheckFilesize(hold) < 4000000)
            {
                m_Ofs.open(hold.c_str(), std::ofstream::out | std::ofstream::app);
                if(m_Ofs.is_open())
                {
                    m_Filepath = filepath;
                    // Check size

                }
                break;
            }
            else
            {
                hold = filepath;
                char buf[100]={'\0'};
                sprintf(buf, "%d", namecount);
                hold.append(buf);
                namecount++;
                //std::cout<<" filepath : " << filepath << std::endl;
            }

        }
    }
}

void SyncBuffer::Shutdown()
{
    if(m_Ofs.is_open())
        m_Ofs.close();

    if(m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

SyncBuffer* SyncBuffer::GetInstance()
{
    if(!m_pInstance)
        m_pInstance = new SyncBuffer();
    return m_pInstance;
}

void SyncBuffer::PushToBuffer(std::ostringstream& os)
{
    Lock();
    if(m_Ofs.is_open())
    {
        m_Ofs << os.str() << std::endl;
        os.flush();
    }
    Unlock();
}

Logger::Logger()
{
    m_pSyncBuffer = SyncBuffer::GetInstance();
}

Logger::~Logger()
{

}

std::string Logger::ToString(Logger::LogLevel level)
{
    static const char* const buffer[] = {"UNKNOWN","ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1" };
    if(level < 6)
        return buffer[level];
    return buffer[0];
}

void Logger::Log(const Logger::LogLevel level, const std::string& input)
{
    os << TimeNow();
    os << " - " << ToString(level) << " : " << input ;
    if(m_pSyncBuffer)
        m_pSyncBuffer->PushToBuffer(os);
}

void Logger::PrintBuffer()
{
    std::cout<< os.str() << std::endl;
}

