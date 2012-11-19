
#include "connectionmanager.h"

#include <string.h>
#include <stdlib.h>

#include <iostream>
struct tdata {
    char *ptr;
    size_t len;
};
static size_t WriteOutFunc(void *ptr, size_t size, size_t nmemb, struct tdata *s);
static void InitDataObject(struct tdata *s);
static tdata* CreateDataObject();
static void DestroyDataObject(tdata* pData);
static std::string ExtractDataToString(tdata* pData);

ConnectionManager* ConnectionManager::m_pInstance = 0;

ConnectionManager::ConnectionManager()
{

}

ConnectionManager::~ConnectionManager()
{
    ///if(m_pInstance)
      //  m_pInstance->Shutdown();
}

ConnectionManager* ConnectionManager::GetInstance()
{
    if(!m_pInstance)
    {
        m_pInstance = new ConnectionManager();
        m_pInstance->Initialize();
    }

    return m_pInstance;
}

void ConnectionManager::Initialize()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_pCurl = curl_easy_init();
}

void ConnectionManager::Shutdown()
{
    if(m_pCurl)
        curl_easy_cleanup(m_pCurl);

    curl_global_cleanup();


    if(m_pInstance)
        delete m_pInstance;
}

std::string ConnectionManager::HttpGet(std::string &url)
{
    std::string response;

    if(m_pCurl)
    {
        CURLcode res; 
        tdata* s = CreateDataObject();

        curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
        //curl_easy_setopt(m_pCurl, CURLOPT_NOBODY, 1);
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc);
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, s);

        res = curl_easy_perform(m_pCurl);

        if(res != CURLE_OK)
        {
            std::cout<<"ERRR"<<std::endl;
            response.append("ERR");
            return response;
        }

        response.clear();
        response.append(ExtractDataToString(s));
        DestroyDataObject(s);
    }

    return response;
}

////////////////////////////////////////////////////////////////////////////////
// Curl Utility functions
////////////////////////////////////////////////////////////////////////////////
//
static size_t WriteOutFunc(void *ptr, size_t size, size_t nmemb, struct tdata *s)
{
    size_t new_len = s->len + size*nmemb;

    if(s->ptr)
    {
        delete s->ptr;
        s->ptr = 0;
    }

    s->ptr = new char[new_len+1];

    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr, size*nmemb);

    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

static void InitDataObject(struct tdata *s)
{
    s->len = 0;
    s->ptr = new char[(s->len+1)];
    if (s->ptr == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    s->ptr[0] = '\0';
}

static tdata* CreateDataObject() 
{
    tdata* pData = new tdata;
    InitDataObject(pData);

    return pData;
}

static void DestroyDataObject(tdata* pData)
{
    if(pData)
    {
        if(pData->ptr)
        {
            delete pData->ptr;
            pData->ptr = 0;
        }

        delete pData;
        pData = 0;
    }
}

static std::string ExtractDataToString(tdata* pData)
{
    std::string str;

    if(pData && pData->ptr)
    {
        str.append(pData->ptr, pData->len);
    }

    return str;
}

