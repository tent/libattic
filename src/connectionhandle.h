#ifndef CONNECTIONHANDLE_H_
#define CONNECTIONHANDLE_H_
#pragma once

#include <iostream>
#include <stdio.h>
#include <curl/curl.h>

class ConnectionHandle
{
public:
    ConnectionHandle() 
    {
        m_pCurlHandle = NULL;
        m_pMultiHandle = NULL;
    }

    ~ConnectionHandle() 
    {
        Cleanup();
    }

    void Init() { m_pCurlHandle = curl_easy_init(); }
    void InitMulti() { m_pMultiHandle = curl_multi_init(); }

    void Cleanup() 
    {
        if(m_pCurlHandle)
        {
            curl_easy_cleanup(m_pCurlHandle);
            m_pCurlHandle = NULL;
        }

        if(m_pMultiHandle)
        {
            curl_multi_cleanup(m_pMultiHandle);
            m_pMultiHandle = NULL;
        }
    }

    double GetUploadSpeed()
    {
        double speed_upload, total_time;
        curl_easy_getinfo(m_pCurlHandle, CURLINFO_SPEED_UPLOAD, &speed_upload);
        curl_easy_getinfo(m_pCurlHandle, CURLINFO_TOTAL_TIME, &total_time);
        return (speed_upload / total_time);
    }

    double GetDownloadSpeed()
    {
        double speed_download, total_time;
        curl_easy_getinfo(m_pCurlHandle, CURLINFO_SPEED_DOWNLOAD, &speed_download);
        curl_easy_getinfo(m_pCurlHandle, CURLINFO_TOTAL_TIME, &total_time);
        return (speed_download / total_time);
    }

    void GetSpeedInfo()
    {
        double speed_upload, total_time;
        curl_easy_getinfo(m_pCurlHandle, CURLINFO_SPEED_UPLOAD, &speed_upload);
        curl_easy_getinfo(m_pCurlHandle, CURLINFO_TOTAL_TIME, &total_time);

        std::cout<<" speed : " << speed_upload << std::endl;
        std::cout<<" time : " << total_time << std::endl;
        printf("Speed: %.3f bytes/sec during %.3f seconds\n", speed_upload, total_time);
    }
    

    CURL* GetHandle() { return m_pCurlHandle; }
    CURLM* GetMultiHandle() { return m_pMultiHandle; }
private:
    CURL*   m_pCurlHandle;
    CURLM*  m_pMultiHandle; 
};

#endif

