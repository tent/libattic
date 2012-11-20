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

struct WriteOut
{
    const char *readptr;
    int sizeleft;
};

static size_t WriteOutFunc(void *ptr, size_t size, size_t nmemb, struct tdata *s);
static void InitDataObject(struct tdata *s);
static tdata* CreateDataObject();
static void DestroyDataObject(tdata* pData);
static std::string ExtractDataToString(tdata* pData);
static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms);
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);

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

void ConnectionManager::HttpPost(const std::string &url, const std::string &body, std::string &responseOut, bool verbose)
{
    if(m_pCurl)
    {
        WriteOut postd; // Post content to be read
        postd.readptr = body.c_str(); // serialized json (should be)
        postd.sizeleft = body.size();

        CURLcode res; 
        tdata* s = CreateDataObject();

        if(verbose)
            curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1L);   
        
        curl_slist *headers = 0; // Init to null, always
        headers = curl_slist_append(headers, "Accept: application/vnd.tent.v0+json" );
        headers = curl_slist_append(headers, "Content-Type: application/vnd.tent.v0+json");

        // Set url
        curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
        // Set that we want to Post
        curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
        // Set the read function
        curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, read_callback);

        // Set Post data 
        curl_easy_setopt(m_pCurl, CURLOPT_READDATA, &postd);
        curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, postd.sizeleft);

        // Write out headers 
        curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

        // Set read response func and data
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc); 
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, s); 
        
        res = curl_easy_perform(m_pCurl);

        if(res != CURLE_OK)
        {
            std::cout<<"Post failed... " << curl_easy_strerror(res) << std::endl;
        }

        responseOut.clear();
        responseOut.append(ExtractDataToString(s));
        DestroyDataObject(s);
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

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct tdata *pooh = (struct tdata *)userp;

    if(size*nmemb < 1)
        return 0;
    if(pooh->len)
    {
        *(char *)ptr = pooh->ptr[0]; /* copy one single byte */ 
        pooh->ptr++;                 /* advance pointer */ 
        pooh->len--;                /* less data left */ 
        return 1;                        /* we return 1 byte at a time! */ 
    }

    return 0;                          /* no more data left to deliver */ 
}
