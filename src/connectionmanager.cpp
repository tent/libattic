#include "connectionmanager.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

struct tdata
{
    char *ptr;
    size_t len;
};

static size_t WriteOutFunc(void *ptr, size_t size, size_t nmemb, struct tdata *s);
static void InitDataObject(struct tdata *s);
static tdata* CreateDataObject();
static void DestroyDataObject(tdata* pData);
static std::string ExtractDataToString(tdata* pData);
static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms);

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

void ConnectionManager::HttpPost(const std::string &url, const std::string &body)
{
    if(m_pCurl)
    {
        CURLcode res; 
        curl_socket_t sockfd;
        long sockextr;
        size_t iolen;


        curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());

        // Connect to host, for socket extraction
        curl_easy_setopt(m_pCurl, CURLOPT_CONNECT_ONLY, 1L);
        res = curl_easy_perform(m_pCurl);

        if(res != CURLE_OK)
        {
            std::cout<<"ERROR : " << curl_easy_strerror(res) << std::endl;
            return;
        }

       /* Extract the socket from the curl handle - we'll need it for waiting.
        * Note that this API takes a pointer to a 'long' while we use
        * curl_socket_t for sockets otherwise.
        */  
        res = curl_easy_getinfo(m_pCurl, CURLINFO_LASTSOCKET, &sockextr);

        if(res != CURLE_OK)
        {
            std::cout<<"ERROR : " << curl_easy_strerror(res) << std::endl;
            return;
        }

        sockfd = sockextr;

        /* wait for the socket to become available for sending*/
        if(!wait_on_socket(sockfd, 0, 60000L))
        {
            std::cout<<"ERROR : timeout\n";
            return;
        }

        /* send the request */

        res = curl_easy_send(m_pCurl, body.c_str(), body.size(), &iolen);
        
        if(res != CURLE_OK)
        {
            std::cout<<"ERROR : " << curl_easy_strerror(res) << std::endl;
            return;
        }

        /* read response */
        curl_off_t nread;
        for(;;)
        {
            char buf[1024];
            wait_on_socket(sockfd, 1, 60000L);
            res = curl_easy_recv(m_pCurl, buf, 1024, &iolen);

            if(res != CURLE_OK)
            {
                std::cout<<"ERROR : " << curl_easy_strerror(res) << std::endl;
                break;
            }

            nread = (curl_off_t)iolen;
            printf("Received %" CURL_FORMAT_CURL_OFF_T " bytes.\n", nread);
        }
    }
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

/* Auxiliary function that waits on the socket. */ 
static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms)
{
    struct timeval tv;
    fd_set infd, outfd, errfd;
    int res;

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec= (timeout_ms % 1000) * 1000;

    FD_ZERO(&infd);
    FD_ZERO(&outfd);
    FD_ZERO(&errfd);
     
    FD_SET(sockfd, &errfd); /* always check for error */ 

    if(for_recv)
    {
        FD_SET(sockfd, &infd);
    }
    else
    {
        FD_SET(sockfd, &outfd);
    }

    /* select() returns the number of signalled sockets or -1 */ 
    res = select(sockfd + 1, &infd, &outfd, &errfd, &tv);
    return res;
}
