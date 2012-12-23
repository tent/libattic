#include "connectionmanager.h"

#include <iostream>
#include <sstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <hex.h>
#include <hmac.h>

#include "url.h"
#include "urlparams.h"
#include "utils.h"
#include "errorcodes.h"

struct WriteOut
{
    const char *readptr;
    int sizeleft;
};

static size_t WriteOutFunc( void *ptr, 
                            size_t size, 
                            size_t nmemb, 
                            struct tdata *s);

static int wait_on_socket( curl_socket_t sockfd, 
                           int for_recv, 
                           long timeout_ms);

static size_t WriteOutToString( void *ptr, 
                                size_t size, 
                                size_t nmemb, 
                                std::string *s);

static size_t write_data( void *ptr, 
                          size_t size, 
                          size_t nmemb, 
                          FILE *stream);

ConnectionManager* ConnectionManager::m_pInstance = 0;

ConnectionManager::ConnectionManager()
{

}

ConnectionManager::~ConnectionManager()
{
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
    utils::SeedRand();
}

void ConnectionManager::Shutdown()
{
    if(m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

void ConnectionManager::EncodeAndAppendUrlParams( CURL* pCurl, 
                                                  const UrlParams* pParams, 
                                                  std::string &url)
{
    if(pCurl && pParams)
    {
        std::string params;  
        //pParams->SerializeToString(params);
        pParams->SerializeAndEncodeToString(pCurl, params);

        //char *pPm = curl_easy_escape(pCurl, params.c_str() , params.size());
        //url.append(pPm);
        url.append(params);
        std::cout << " URL APPEND : " << url << std::endl;
    }
}


int ConnectionManager::HttpDelete( const std::string& url,
                                   const UrlParams* pParams,
                                   Response& responseOut,
                                   const std::string& macalgorithm, 
                                   const std::string& macid, 
                                   const std::string& mackey, 
                                   bool verbose)
{

    CURL* pCurl = curl_easy_init();
    CURLcode res; 

    curl_slist *headers = 0; // Init to null, always

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    std::string authheader;
    BuildAuthHeader( urlPath, 
                     std::string("DELETE"), 
                     macid, 
                     mackey, 
                     authheader);

    headers = curl_slist_append(headers, authheader.c_str());

    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    //curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "DELETE"); 

    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut.body);

    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        return ret::A_FAIL_CURL_PERF;
    }

    responseOut.code = GetResponseCode(pCurl);

    curl_easy_cleanup(pCurl);
    return ret::A_OK;
}

int ConnectionManager::GetResponseCode(CURL* pCurl)
{
    if(!pCurl)
        return -1;

    long responsecode = 0;
    curl_easy_getinfo (pCurl, CURLINFO_RESPONSE_CODE, &responsecode);

    return responsecode; 
}

int ConnectionManager::HttpGet( const std::string &url, 
                                const UrlParams* pParams,
                                Response& responseOut, 
                                bool verbose)
{

    CURL* pCurl = curl_easy_init();
    CURLcode res; 

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

    curl_slist *headers = 0; // Init to null, always
    headers = curl_slist_append(headers, "Accept: application/vnd.tent.v0+json" );
    headers = curl_slist_append(headers, "Content-Type: application/vnd.tent.v0+json");

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut.body);

    // Write out headers 
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        std::cout<<"ERRR"<<std::endl;
        return ret::A_FAIL_CURL_PERF;
    }

    responseOut.code = GetResponseCode(pCurl);
    curl_easy_cleanup(pCurl);

    return ret::A_OK;
}

int ConnectionManager::HttpGetWithAuth( const std::string& url, 
                                        const UrlParams* pParams,
                                        Response& responseOut, 
                                        const std::string& macalgorithm, 
                                        const std::string& macid, 
                                        const std::string& mackey, 
                                        bool verbose)
{

    CURL* pCurl = curl_easy_init();
    CURLcode res; 

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    curl_slist *headers = 0; // Init to null, always
    // TODO :: ABSTRACT HEADERS OUT
    headers = curl_slist_append(headers, "Accept: application/vnd.tent.v0+json" );
    //headers = curl_slist_append(headers, "Accept: binary" );
    headers = curl_slist_append(headers, "Content-Type: application/vnd.tent.v0+json");
    //headers = curl_slist_append(headers, "Content-Type: binary");

    std::string authheader;
    BuildAuthHeader( urlPath, 
                     std::string("GET"), 
                     macid, 
                     mackey,
                     authheader);

    headers = curl_slist_append(headers, authheader.c_str());

    std::cout << " GETTING URL " << url << std::endl;
    std::cout << " URLPATH : " <<urlPath << std::endl;
    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    //curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);
    
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut.body);

    // Write out headers 
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);


    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        return ret::A_FAIL_CURL_PERF;
    }

    responseOut.code = GetResponseCode(pCurl);
    curl_slist_free_all (headers);
    curl_easy_cleanup(pCurl);

    return ret::A_OK;
}

int ConnectionManager::HttpGetAttachmentWriteToFile( const std::string &url, 
                                                     const UrlParams* pParams,
                                                     Response& responseOut,
                                                     const std::string &filepath, 
                                                     const std::string &macalgorithm, 
                                                     const std::string &macid, 
                                                     const std::string &mackey, 
                                                     bool verbose)
{
    CURL* pCurl = curl_easy_init();
    CURLcode res; 

    // Write out to file
    FILE *fp;
    fp = fopen(filepath.c_str(), "wb");

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    curl_slist *headers = 0; // Init to null, always
    headers = curl_slist_append(headers, "Accept: application/octet-stream" );

    // Build Auth header
    std::string authheader;
    BuildAuthHeader( urlPath, 
                     std::string("GET"), 
                     macid, 
                     mackey, 
                     authheader);

    headers = curl_slist_append(headers, authheader.c_str());

    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

    // Write out headers 
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        std::cout<<"ERRR"<<std::endl;
        return ret::A_FAIL_CURL_PERF;
    }
    fclose(fp);

    responseOut.code = GetResponseCode(pCurl);
    curl_slist_free_all (headers);
    curl_easy_cleanup(pCurl);

    return ret::A_OK;
}

int ConnectionManager::HttpPost( const std::string& url, 
                                 const UrlParams* pParams,
                                 const std::string& body, 
                                 Response& responseOut, 
                                 bool verbose)
{

    CURL* pCurl = curl_easy_init();
    WriteOut postd; // Post content to be read
    postd.readptr = body.c_str(); // serialized json (should be)
    postd.sizeleft = body.size();

    CURLcode res; 

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);   

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    curl_slist *headers = 0; // Init to null, always
    headers = curl_slist_append( headers, 
                                 "Accept: application/vnd.tent.v0+json" );

    headers = curl_slist_append( headers, 
                                 "Content-Type: application/vnd.tent.v0+json");

    // Set url
    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    // Set that we want to Post
    curl_easy_setopt(pCurl, CURLOPT_POST, 1L);

    // Set Post data 
    curl_easy_setopt(pCurl, CURLOPT_READDATA, &postd);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, postd.sizeleft);

    // Write out headers 
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

    // Set read response func and data
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString); 
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut.body); 
    
    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        std::cout<<"Post failed... " << curl_easy_strerror(res) << std::endl;
        return ret::A_FAIL_CURL_PERF;
    }
 
    responseOut.code = GetResponseCode(pCurl);
    curl_easy_cleanup(pCurl);

    return ret::A_OK;
}

int ConnectionManager::HttpMultipartPut( const std::string &url, 
                                         const UrlParams* pParams,
                                         const std::string &body, 
                                         std::list<std::string>* filepaths, 
                                         Response& responseOut, 
                                         const std::string &macalgorithm, 
                                         const std::string &macid, 
                                         const std::string &mackey, 
                                         bool verbose)
{
    // Send Multiple files as attachments
    CURL* pCurl = 0;
    CURLM *multi_handle = 0;
    int still_running = 0;

    curl_slist      *headerlist=NULL;
    curl_httppost   *formpost=NULL;
    curl_httppost   *lastptr=NULL;

    static const char szExpectBuf[] = "Expect:";

    curl_slist *partlist = 0;

    curl_slist      **pl = &partlist;

    curl_httppost   **fp = &formpost;
    curl_httppost   **lp = &lastptr;

    AddBodyToForm( body,
                   fp,
                   lp,
                   pl );

    /*
    // Fill out json header
    partlist = curl_slist_append( partlist, 
                                  "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"");

    char szLen[256];
    memset(szLen, '\0', sizeof(char)*256);                                              
    snprintf(szLen, (sizeof(char)*256),  "%lu", body.size());     

    std::string jcl("Content-Length: ");
    jcl.append(szLen);

    partlist = curl_slist_append(partlist, jcl.c_str());
    partlist = curl_slist_append(partlist, "Content-Type: application/vnd.tent.v0+json");
    partlist = curl_slist_append(partlist, "Content-Transfer-Encoding: binary");

    curl_formadd( &formpost,
                  &lastptr,
                  CURLFORM_COPYNAME, "jsonbody",
                  CURLFORM_COPYCONTENTS, body.c_str(),
                  CURLFORM_CONTENTHEADER, partlist,
                  CURLFORM_END);
    
                  */
    // Add Attachment(s)
    curl_slist *attachlist = 0;
    curl_slist **al = &attachlist;

    std::list<std::string>::iterator itr = filepaths->begin();
    // Go through file 
    for(; itr != filepaths->end(); itr++) 
    {
        std::cout<<"Adding attachment ... " << std::endl;
        
        AddAttachmentToForm( *itr, 
                             fp,
                             lp,
                             al);
    }
    
    pCurl = curl_easy_init();
    multi_handle = curl_multi_init();
   
    /* initalize custom header list (stating that Expect: 100-continue is not
    *      wanted */ 

    headerlist = curl_slist_append(headerlist, szExpectBuf);

    if(pCurl && multi_handle)
    {
        if(verbose)
            curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);   
            
        std::string urlPath = url;
        EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

        curl_slist *headers=NULL;
         
        headerlist = curl_slist_append(headerlist, "Accept: application/vnd.tent.v0+json" );
        headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data");
        //headerlist = curl_slist_append(headerlist, "Content-Type: application/vnd.tent.v0+json");

        std::string authheader;
        BuildAuthHeader( urlPath, 
                         std::string("PUT"), 
                         macid, 
                         mackey, 
                         authheader);

        headerlist = curl_slist_append(headerlist, authheader.c_str());

        /* what URL that receives this POST */ 
        curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PUT");

        curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);

        curl_multi_add_handle(multi_handle, pCurl);

        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut.body); 
    
        curl_multi_perform(multi_handle, &still_running);

        /* lets start the fetch */
        do {
                struct timeval timeout;
                int rc; /* select() return code */ 
             
                fd_set fdread;
                fd_set fdwrite;
                fd_set fdexcep;
                int maxfd = -1;
                                      
                long curl_timeo = -1;
                                             
                FD_ZERO(&fdread);
                FD_ZERO(&fdwrite);
                FD_ZERO(&fdexcep);
                                                           
                /* set a suitable timeout to play around with */ 
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                       
                curl_multi_timeout(multi_handle, &curl_timeo);
                if(curl_timeo >= 0) {
                   timeout.tv_sec = curl_timeo / 1000;
                   if(timeout.tv_sec > 1)
                        timeout.tv_sec = 1;
                   else
                        timeout.tv_usec = (curl_timeo % 1000) * 1000;
                 }
                                                                                                                                  
                /* get file descriptors from the transfers */ 
                curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

                /* In a real-world program you OF COURSE check the return code of the
                function calls.  On success, the value of maxfd is guaranteed to be
                greater or equal than -1.  We call select(maxfd + 1, ...), specially in
                case of (maxfd == -1), we call select(0, ...), which is basically equal
                to sleep. */ 

                rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

                switch(rc) {
                case -1:
                /* select error */ 
                break;
                case 0:
                default:
                /* timeout or readable/writable sockets */ 
                printf("perform!\n");
                curl_multi_perform(multi_handle, &still_running);
                printf("running: %d!\n", still_running);
                break;
                }
            } while(still_running);

/*
        do {
                while(::curl_multi_perform(multi_handle, &still_running) ==
                                CURLM_CALL_MULTI_PERFORM);
        } while (still_running);
*/

        responseOut.code = GetResponseCode(pCurl);
        curl_multi_cleanup(multi_handle);

        /* then cleanup the formpost chain */ 
        curl_formfree(formpost);
                                                           
        /* free slist */ 
        curl_slist_free_all (headerlist);
        curl_slist_free_all (partlist);
        curl_slist_free_all (attachlist);
    }

    curl_easy_cleanup(pCurl);

    return ret::A_OK;
}

int ConnectionManager::HttpMultipartPost( const std::string &url, 
                                          const UrlParams* pParams,
                                          const std::string &body, 
                                          std::list<std::string>* filepaths, 
                                          Response& responseOut, 
                                          const std::string &macalgorithm, 
                                          const std::string &macid, 
                                          const std::string &mackey, 
                                          bool verbose)
{
    // Send Multiple files as attachments
    CURL* pCurl = 0;
    CURLM *multi_handle = 0;
    int still_running = 0;

    curl_slist      *headerlist=NULL;
    curl_httppost   *formpost=NULL;
    curl_httppost   *lastptr=NULL;

    static const char szExpectBuf[] = "Expect:";

    curl_slist *partlist = 0;
    
    curl_slist      **pl = &partlist; 
    curl_httppost   **fp = &formpost;
    curl_httppost   **lp = &lastptr;


    AddBodyToForm( body,
                   fp,
                   lp,
                   pl );


    // Add Attachment(s)
    std::list<std::string>::iterator itr = filepaths->begin();
    // Go through file 
    curl_slist *attachlist = 0;

    curl_slist **al = &attachlist;
    
    for(; itr != filepaths->end(); itr++) 
    {
                std::cout<<"Adding attachment ... " << std::endl;
        AddAttachmentToForm( *itr, 
                             fp,
                             lp,
                             al);
   }
    
    pCurl = curl_easy_init();
    multi_handle = curl_multi_init();
   
    /* initalize custom header list (stating that Expect: 100-continue is not
    *      wanted */ 

    headerlist = curl_slist_append(headerlist, szExpectBuf);

    if(pCurl && multi_handle)
    {
        if(verbose)
            curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);   
            
        std::string urlPath = url;
        EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

        curl_slist *headers=NULL;
         
        headerlist = curl_slist_append(headerlist, "Accept: application/vnd.tent.v0+json" );
        headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data");
        //headerlist = curl_slist_append(headerlist, "Content-Type: application/vnd.tent.v0+json");

        std::string authheader;
        BuildAuthHeader( urlPath, 
                         std::string("POST"), 
                         macid, 
                         mackey, 
                         authheader);

        headerlist = curl_slist_append(headerlist, authheader.c_str());

        /* what URL that receives this POST */ 
        curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);

        curl_multi_add_handle(multi_handle, pCurl);

        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut.body); 
    
        curl_multi_perform(multi_handle, &still_running);

        /* lets start the fetch */
        do {
                struct timeval timeout;
                int rc; /* select() return code */ 
             
                fd_set fdread;
                fd_set fdwrite;
                fd_set fdexcep;
                int maxfd = -1;
                                      
                long curl_timeo = -1;
                                             
                FD_ZERO(&fdread);
                FD_ZERO(&fdwrite);
                FD_ZERO(&fdexcep);
                                                           
                /* set a suitable timeout to play around with */ 
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                       
                curl_multi_timeout(multi_handle, &curl_timeo);
                if(curl_timeo >= 0) {
                   timeout.tv_sec = curl_timeo / 1000;
                   if(timeout.tv_sec > 1)
                        timeout.tv_sec = 1;
                   else
                        timeout.tv_usec = (curl_timeo % 1000) * 1000;
                 }
                                                                                                                                  
                /* get file descriptors from the transfers */ 
                curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

                /* In a real-world program you OF COURSE check the return code of the
                function calls.  On success, the value of maxfd is guaranteed to be
                greater or equal than -1.  We call select(maxfd + 1, ...), specially in
                case of (maxfd == -1), we call select(0, ...), which is basically equal
                to sleep. */ 

                rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

                switch(rc) {
                case -1:
                /* select error */ 
                break;
                case 0:
                default:
                /* timeout or readable/writable sockets */ 
                printf("perform!\n");
                curl_multi_perform(multi_handle, &still_running);
                printf("running: %d!\n", still_running);
                break;
                }
            } while(still_running);

/*
        do {
                while(::curl_multi_perform(multi_handle, &still_running) ==
                                CURLM_CALL_MULTI_PERFORM);
        } while (still_running);
*/

        responseOut.code = GetResponseCode(pCurl);
        curl_multi_cleanup(multi_handle);

        /* then cleanup the formpost chain */ 
        curl_formfree(formpost);
                                                           
        /* free slist */ 
        curl_slist_free_all (headerlist);
        curl_slist_free_all (partlist);
        //curl_slist_free_all (attachlist);
    }

    curl_easy_cleanup(pCurl);

    return ret::A_OK;
}

int ConnectionManager::HttpPostWithAuth( const std::string &url, 
                                          const UrlParams* pParams,
                                          const std::string &body, 
                                          Response &responseOut, 
                                          const std::string &macalgorithm, 
                                          const std::string &macid, 
                                          const std::string &mackey, 
                                          bool verbose)
{

    CURL* pCurl = curl_easy_init();
    WriteOut postd; // Post content to be read
    postd.readptr = body.c_str(); // serialized json (should be)
    postd.sizeleft = body.size();

    CURLcode res; 

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);   

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    curl_slist *headers = 0; // Init to null, always
    headers = curl_slist_append(headers, "Accept: application/vnd.tent.v0+json" );
    headers = curl_slist_append(headers, "Content-Type: application/vnd.tent.v0+json");

    std::string authheader;
    BuildAuthHeader( urlPath, 
                     std::string("POST"), 
                     macid, 
                     mackey, 
                     authheader);

    headers = curl_slist_append(headers, authheader.c_str());

    // Set url
    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    // Set that we want to Post
    curl_easy_setopt(pCurl, CURLOPT_POST, 1L);

    // Set Post data 
    curl_easy_setopt(pCurl, CURLOPT_READDATA, &postd);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, postd.sizeleft);

    // Write out headers 
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

    // Set read response func and data
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString); 
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut.body); 
    
    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        std::cout<<"Post failed... " << curl_easy_strerror(res) << std::endl;
        return ret::A_FAIL_CURL_PERF;
    }

    responseOut.code = GetResponseCode(pCurl);
    curl_easy_cleanup(pCurl);

    return ret::A_OK;

}

void ConnectionManager::AddBodyToForm( const std::string &body,
                                              curl_httppost **post, 
                                              curl_httppost **last, 
                                              curl_slist **list)
{
    // Fill out json header
    *list = curl_slist_append( *list, 
                              "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"");

    std::cout<< "BODY : " << body << std::endl;
    char szLen[256];
    memset(szLen, '\0', sizeof(char)*256);                                              
    snprintf(szLen, (sizeof(char)*256),  "%lu", body.size());     

    std::string jcl("Content-Length: ");
    jcl.append(szLen);

    *list = curl_slist_append(*list, jcl.c_str());
    *list = curl_slist_append(*list, "Content-Type: application/vnd.tent.v0+json");
    *list = curl_slist_append(*list, "Content-Transfer-Encoding: binary");

    curl_formadd( &*post,
                  &*last,
                  CURLFORM_COPYNAME, "jsonbody",
                  CURLFORM_COPYCONTENTS, body.c_str(),
                  CURLFORM_CONTENTHEADER, *list,
                  CURLFORM_END);

}

void ConnectionManager::AddAttachmentToForm( const std::string &path, 
                                                    curl_httppost **post, 
                                                    curl_httppost **last, 
                                                    curl_slist **list)
{
    // Addes an attachment as a part to the form

    // Extract Filename
    std::string name;
    utils::ExtractFileName(path, name);

    // Get Filesize
    unsigned int uSize = utils::CheckFilesize(path);

    // Gather some information about the content
    std::string cd;
    cd.append("Content-Disposition: form-data; name=\"attach\"; filename=\"");
    cd += name;
    cd.append("\"");

    *list = curl_slist_append(*list, "Content-Transfer-Encoding: binary");
    *list = curl_slist_append(*list, cd.c_str());
    
    // Append size to a string
    char szBuffer[256];
    memset(szBuffer, '\0', sizeof(char)*256);                                              
    snprintf(szBuffer, (sizeof(char)*256),  "%d", uSize);     

    std::string cl("Content-Length: ");
    cl.append(szBuffer);

    std::cout<< "CL : " << cl <<std::endl;
    std::cout<< "USIZE : " << uSize << std::endl;
        
    *list = curl_slist_append(*list,  cl.c_str());
    *list = curl_slist_append(*list, "Content-Type: application/octet-stream");

    std::cout << " name : " << name << std::endl;
    std::cout << " path : " << path << std::endl;

    curl_formadd( &*post, 
                  &*last, 
                  CURLFORM_COPYNAME, name.c_str(),
                  CURLFORM_FILE, path.c_str(), 
                  CURLFORM_CONTENTHEADER, *list,
                  CURLFORM_END);     

}


void ConnectionManager::BuildAuthHeader( const std::string &url, 
                                         const std::string &requestMethod, 
                                         const std::string &macid, 
                                         const std::string &mackey, 
                                         std::string& out)
{
    std::string n;
    GenerateNonce(n);

    out.clear();
    out.append("Authorization: ");
    
    out.append("MAC id=\"");
    out.append(macid.c_str());
    out.append("\", ");

    time_t t = time(0);
    char tb[256];
    snprintf(tb, (sizeof(time_t)*256), "%ld", t);

    out.append("ts=\"");
    out.append(tb);
    out.append("\", ");

    out.append("nonce=\"");
    out.append(n);
    out.append("\", ");

    
    Url u(url);

    std::string port;
    if(u.HasPort())
        port = u.GetPort();
    else
    {
        if(u.GetScheme().compare(std::string("https")))
            port.append("443");
        else
            port.append("443");
    }

    std::string requestString;
    requestString.append(tb); // time 
    requestString.append("\n");
    requestString.append(n); // nonce 
    requestString.append("\n");
    requestString.append(requestMethod); // method
    requestString.append("\n");
    std::string uri;
    u.GetRequestURI(uri);
    requestString.append(uri); // request uri
    requestString.append("\n");
    requestString.append(u.GetHost()); // host
    requestString.append("\n");
    requestString.append(port); // port
    requestString.append("\n\n");

    std::string signedreq;
    SignRequest(requestString, mackey, signedreq);

    out.append("mac=\"");
    out.append(signedreq.c_str());
    out.append("\"");
    

   // std::cout << "REQUEST_STRING : " << requestString << std::endl;
   // std::cout << "AUTH_HEADER : " << out << std::endl;
}

void ConnectionManager::GenerateNonce(std::string &out)
{
    out.clear();
    std::string seed;
    for(int i=0; i<3; i++)
        seed+=utils::GenerateChar();

    utils::StringToHex(seed, out);
}

#include <string.h>
#include <stdio.h>
#include <base64.h>
void ConnectionManager::SignRequest( const std::string &request, 
                                     const std::string &key, 
                                     std::string &out)
{
    std::string mac, encoded, som;

    try
    {
  //      std::cout<< "Key : " << key << std::endl;
        unsigned char szReqBuffer[request.size()];
        memcpy(szReqBuffer, key.c_str(), strlen(key.c_str())+1);

 //       std::cout<< " BUFFER : " << szReqBuffer << std::endl;
        //CryptoPP::HMAC< CryptoPP::SHA256 > hmac(szReqBuffer, request.size());

        CryptoPP::HMAC< CryptoPP::SHA256 > hmac(szReqBuffer, strlen(key.c_str())+1);

        CryptoPP::StringSource( request,
                                true, 
                                new CryptoPP::HashFilter(hmac,
                                new CryptoPP::StringSink(mac)
                               ) // HashFilter      
                    ); // StringSource

        CryptoPP::StringSource( mac,
                                true,
                                new CryptoPP::Base64Encoder(
                                new CryptoPP::StringSink(som),
                                false));
    }
    catch(const CryptoPP::Exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    // Hex encoding, ignore this for now
    encoded.clear();

    CryptoPP::StringSource( som, 
                  true,
                  new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)) // HexEncoder
                ); // StringSource


//    std::cout << "hmac: " << encoded << std::endl; 
//    std::cout << "mac : " << mac << std::endl;

    // trim
    size_t found = som.find(std::string("="));
    if (found != std::string::npos)
    {
        som = som.substr(0, found+1);
    }

    out.clear();
    //out = encoded;
    out = som;
}

void ConnectionManager::GenerateHmacSha256(std::string &out)
{
    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::SecByteBlock key(16);
    prng.GenerateBlock(key, key.size());

    std::string plain = "HMAC Test";
    std::string mac, encoded;

    /*********************************\
    \*********************************/

    encoded.clear();

    CryptoPP::StringSource( key, 
                  key.size(), 
                  true,
                  new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)) // HexEncoder
                ); // StringSource

    std::cout << "key: " << encoded << std::endl;
    std::cout << "plain text: " << plain << std::endl;

    /*********************************\
    \*********************************/

    try
    {
        CryptoPP::HMAC< CryptoPP::SHA256 > hmac(key, key.size());

        CryptoPP::StringSource( plain, 
                      true, 
                      new CryptoPP::HashFilter( hmac,
                                      new CryptoPP::StringSink(mac)
                                    ) // HashFilter      
                    ); // StringSource
    }
    catch(const CryptoPP::Exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    /*********************************\
    \*********************************/

    // Pretty print
    encoded.clear();

    CryptoPP::StringSource( mac, 
                  true,
                  new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)) // HexEncoder
                ); // StringSource

    std::cout << "hmac: " << encoded << std::endl; 

    out.clear();
    out = encoded;

}


////////////////////////////////////////////////////////////////////////////////
// Curl Utility functions
////////////////////////////////////////////////////////////////////////////////
//

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}


static size_t WriteOutToString(void *ptr, size_t size, size_t nmemb, std::string *s)
{
    unsigned int start_size = s->size();

    std::cout<<"SIZE : " << size << std::endl;
    std::cout<<"NMEMB : " << nmemb << std::endl;
    s->append((char*)ptr, size*nmemb);
    std::cout<<"CURRENT SIZE : " << s->size() << std::endl;

    // Must return the value it wrote out this time;
    return (s->size() - start_size);
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

