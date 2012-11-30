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

static size_t WriteOutFunc( void *ptr, 
                            size_t size, 
                            size_t nmemb, 
                            struct tdata *s);

static void InitDataObject(struct tdata *s);
static tdata* CreateDataObject();
static void DestroyDataObject(tdata* pData);
static std::string ExtractDataToString(tdata* pData);

static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms);

static size_t read_callback( void *ptr, 
                             size_t size, 
                             size_t nmemb, 
                             void *userp);

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
    utils::SeedRand();

//    curl_global_init(CURL_GLOBAL_DEFAULT);
//    pCurl = curl_easy_init();
}

void ConnectionManager::Shutdown()
{
//    if(pCurl)
 //       curl_easy_cleanup(pCurl);

 //   curl_global_cleanup();

    if(m_pInstance)
        delete m_pInstance;
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


void ConnectionManager::HttpDelete( const std::string &url,
                                    const UrlParams* pParams,
                                    std::string &responseOut,
                                    const std::string &szMacAlgorithm, 
                                    const std::string &szMacID, 
                                    const std::string &szMacKey, 
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
                         szMacID, 
                         szMacKey, 
                         authheader);
  
        headers = curl_slist_append(headers, authheader.c_str());

        curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
        //curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);
        curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "DELETE"); 

        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut);

        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(pCurl);

        if(res != CURLE_OK)
        {
            std::cout<<"ERRR"<<std::endl;
            responseOut.append("ERR");
            return;
        }

        curl_easy_cleanup(pCurl);

}


void ConnectionManager::HttpGet( const std::string &url, 
                                 const UrlParams* pParams,
                                 std::string &out, 
                                 bool verbose)
{

    CURL* pCurl = curl_easy_init();
    CURLcode res; 
    tdata* s = CreateDataObject();

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, s);

    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        std::cout<<"ERRR"<<std::endl;
        return;
    }

    out.append(ExtractDataToString(s));
    DestroyDataObject(s);

    curl_easy_cleanup(pCurl);
}

void ConnectionManager::HttpGetWithAuth( const std::string &url, 
                                         const UrlParams* pParams,
                                         std::string &out, 
                                         const std::string &szMacAlgorithm, 
                                         const std::string &szMacID, 
                                         const std::string &szMacKey, 
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
                         szMacID, 
                         szMacKey,
                         authheader);

        headers = curl_slist_append(headers, authheader.c_str());

        std::cout << " GETTING URL " << url << std::endl;
        std::cout << " URLPATH : " <<urlPath << std::endl;
        curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
        //curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);
        
        /*
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc);
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, s);
*/

        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &out);

        // Write out headers 
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);


        res = curl_easy_perform(pCurl);


        std::cout<< "OUT SIZE : " << out.size() << std::endl;

        if(res != CURLE_OK)
        {
            std::cout<<"ERRR"<<std::endl;
            return;
        }



        curl_slist_free_all (headers);

    curl_easy_cleanup(pCurl);
}

void ConnectionManager::HttpGetAttachment( const std::string &url, 
                                           const UrlParams* pParams, 
                                           std::string &out, 
                                           const std::string &szMacAlgorithm, 
                                           const std::string &szMacID, 
                                           const std::string &szMacKey, 
                                           bool verbose)
{

        CURL* pCurl = curl_easy_init();
        CURLcode res; 
        tdata* s = CreateDataObject();

        FILE *fp;

        fp = fopen("ctest", "wb");

        if(verbose)
            curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

        std::string urlPath = url;
        EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

        curl_slist *headers = 0; // Init to null, always
        // TODO :: ABSTRACT HEADERS OUT
        //headers = curl_slist_append(headers, "Accept: application/vnd.tent.v0+json" );
        headers = curl_slist_append(headers, "Accept: application/octet-stream" );
        //headers = curl_slist_append(headers, "Content-Type: application/vnd.tent.v0+json");
        //headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

        std::string authheader;
        BuildAuthHeader(urlPath, std::string("GET"), szMacID, szMacKey, authheader);
        headers = curl_slist_append(headers, authheader.c_str());

        std::cout << " GETTING URL " << url << std::endl;
        curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
        //curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);

        //curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc);
        //curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, s);

        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);

        // Write out headers 
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);


        res = curl_easy_perform(pCurl);

        if(res != CURLE_OK)
        {
            std::cout<<"ERRR"<<std::endl;
            return;
        }

        std::cout<< " S LEN : " << s->len << std::endl;
        std::cout<< " S PTR : " << s->ptr << std::endl;


        //file = fopen("ctest", "wb");
        //fwrite(s->ptr, s->len, 1, file);
        //fprintf(file, "%s", s->ptr);
        fclose(fp);

        curl_slist_free_all (headers);
        out.clear();
        out.append(ExtractDataToString(s));
        DestroyDataObject(s);

    curl_easy_cleanup(pCurl);

}

void ConnectionManager::HttpGetAttachmentWriteToFile( const std::string &url, 
                                                      const UrlParams* pParams,
                                                      const std::string &szFilePath, 
                                                      const std::string &szMacAlgorithm, 
                                                      const std::string &szMacID, 
                                                      const std::string &szMacKey, 
                                                      bool verbose)
{
        CURL* pCurl = curl_easy_init();
        CURLcode res; 

        // Write out to file
        FILE *fp;
        fp = fopen(szFilePath.c_str(), "wb");

        if(verbose)
            curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

        std::string urlPath = url;
        EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

        curl_slist *headers = 0; // Init to null, always
        headers = curl_slist_append(headers, "Accept: application/octet-stream" );

        // Build Auth header
        std::string authheader;
        BuildAuthHeader(urlPath, std::string("GET"), szMacID, szMacKey, authheader);
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
            return;
        }


        fclose(fp);

        curl_slist_free_all (headers);

    curl_easy_cleanup(pCurl);

}


void ConnectionManager::HttpPost( const std::string &url, 
                                  const UrlParams* pParams,
                                  const std::string &body, 
                                  std::string &responseOut, 
                                  bool verbose)
{

    CURL* pCurl = curl_easy_init();
        WriteOut postd; // Post content to be read
        postd.readptr = body.c_str(); // serialized json (should be)
        postd.sizeleft = body.size();

        CURLcode res; 
        tdata* s = CreateDataObject();

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
        // Set the read function
        curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, read_callback);

        // Set Post data 
        curl_easy_setopt(pCurl, CURLOPT_READDATA, &postd);
        curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, postd.sizeleft);

        // Write out headers 
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

        // Set read response func and data
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc); 
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, s); 
        
        res = curl_easy_perform(pCurl);

        if(res != CURLE_OK)
        {
            std::cout<<"Post failed... " << curl_easy_strerror(res) << std::endl;
        }

        responseOut.clear();
        responseOut.append(ExtractDataToString(s));
        DestroyDataObject(s);

    curl_easy_cleanup(pCurl);
}

void ConnectionManager::HttpMultipartPut( const std::string &url, 
                                          const UrlParams* pParams,
                                          const std::string &szBody, 
                                          const std::string &szFilePath, 
                                          const std::string &szFileName,
                                          std::string &responseOut, 
                                          const std::string &szMacAlgorithm, 
                                          const std::string &szMacID, 
                                          const std::string &szMacKey, 
                                          const char* pData,
                                          unsigned int uSize,
                                          bool verbose )
{
    
            CURL* pCurl;
            CURLM *multi_handle;
            
            int still_running = 0;

            curl_httppost *formpost=NULL;
            curl_httppost *lastptr=NULL;
            curl_slist *headerlist=NULL;
            static const char buf[] = "Expect:";
            
            // Add json
            WriteOut postd; // Post content to be read
            postd.readptr = szBody.c_str(); // serialized json (should be)
            postd.sizeleft = szBody.size();

            curl_slist *partlist = 0;

            partlist = curl_slist_append(partlist, "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"");
            partlist = curl_slist_append(partlist, "Content-Length: 206");
            partlist = curl_slist_append(partlist, "Content-Type: application/vnd.tent.v0+json");
            partlist = curl_slist_append(partlist, "Content-Transfer-Encoding: binary");

            curl_formadd( &formpost,
                          &lastptr,
                          CURLFORM_COPYNAME, "jsonbody",
                          CURLFORM_COPYCONTENTS, szBody.c_str(),
                          CURLFORM_CONTENTHEADER, partlist,
                          CURLFORM_END);

            // Read in file 
            curl_slist *attachlist = 0;
            //std::string testBuf("this is my testbuffer");

            std::string cd;
            cd.append("Content-Disposition: form-data; name=\"attach[0]\"; filename=\"");
            cd += szFileName;
            cd.append("\"");

            attachlist = curl_slist_append(attachlist, "Content-Transfer-Encoding: binary");
            attachlist = curl_slist_append(attachlist, cd.c_str());

            char szBuffer[256];
            memset(szBuffer, '\0', sizeof(char)*256);                                              
            snprintf(szBuffer, (sizeof(char)*256),  "%d", uSize);     

            std::string cl("Content-Length: ");
            cl.append(szBuffer);

            if(!pData)
            {
                std::cout<<"INVALID DATA PTR "<< std::endl;
                return;
            }
            
            attachlist = curl_slist_append(attachlist,  cl.c_str());
            attachlist = curl_slist_append(attachlist, "Content-Type: binary");

            curl_formadd( &formpost,
                          &lastptr,
                          CURLFORM_COPYNAME, "attatchment",
                          //CURLFORM_BUFFER, "thisthing",
                          CURLFORM_PTRCONTENTS, pData,
                          CURLFORM_CONTENTSLENGTH, uSize,
                          //CURLFORM_BUFFERLENGTH, uSize,
                          //CURLFORM_BUFFERPTR, pData, 
                          CURLFORM_CONTENTHEADER, attachlist,
                          CURLFORM_END);

            pCurl = curl_easy_init(); 
            multi_handle = curl_multi_init();
           
            /* initalize custom header list (stating that Expect: 100-continue is not
            *      wanted */ 
            headerlist = curl_slist_append(headerlist, buf);

            if(pCurl && multi_handle)
            {
                if(verbose)
                    curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);   

                std::string urlPath = url;
                EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

                struct curl_slist *headers=NULL;
                 
                headerlist = curl_slist_append(headerlist, "Accept: application/vnd.tent.v0+json" );
                headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data");
                //headerlist = curl_slist_append(headerlist, "Content-Type: application/vnd.tent.v0+json");

                std::string authheader;
                BuildAuthHeader(urlPath, std::string("PUT"), szMacID, szMacKey, authheader);
                headerlist = curl_slist_append(headerlist, authheader.c_str());

                /* what URL that receives this POST */ 
                curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
                curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
                curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PUT");

                //curl_easy_setopt(pCurl, CURLOPT_PUT, 1L);

                curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);

                curl_multi_add_handle(multi_handle, pCurl);

                // Set read response func and data
                //curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc); 
                //curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, s); 
            
                std::cout<<"here"<<std::endl;
                curl_multi_perform(multi_handle, &still_running);

                std::cout<<"here"<<std::endl;

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

                /* lets start the fetch */
                /*
                do {
                        while(::curl_multi_perform(multi_handle, &still_running) ==
                                        CURLM_CALL_MULTI_PERFORM);
                } while (still_running);
                */
                std::cout<<"here"<<std::endl;

                //responseOut.clear();
                //responseOut.append(ExtractDataToString(s));
                //DestroyDataObject(s);
                curl_multi_cleanup(multi_handle);

                /* then cleanup the formpost chain */ 
                curl_formfree(formpost);
                                                                   
                /* free slist */ 
                curl_slist_free_all (headerlist);
                curl_slist_free_all (partlist);
                curl_slist_free_all (attachlist);
            }

        curl_easy_cleanup(pCurl);
}


void ConnectionManager::HttpMultipartPost( const std::string &url, 
                                           const UrlParams* pParams,
                                           const std::string &szBody, 
                                           const std::string &szFilePath, 
                                           const std::string &szFileName,
                                           std::string &responseOut, 
                                           const std::string &szMacAlgorithm, 
                                           const std::string &szMacID, 
                                           const std::string &szMacKey, 
                                           const char* pData,
                                           unsigned int uSize,
                                           bool verbose)
{
    

        CURL* pCurl = 0;
        CURLM *multi_handle = 0;
        int still_running = 0;

        curl_slist *headerlist=NULL;
        curl_httppost *formpost=NULL;
        curl_httppost *lastptr=NULL;
        static const char buf[] = "Expect:";

        // Add json
        WriteOut postd; // Post content to be read
        postd.readptr = szBody.c_str(); // serialized json (should be)
        postd.sizeleft = szBody.size();

        curl_slist *partlist = 0;
        
        partlist = curl_slist_append(partlist, "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"");

        char szLen[256];
        memset(szLen, '\0', sizeof(char)*256);                                              
        snprintf(szLen, (sizeof(char)*256),  "%lu", szBody.size());     

        std::string jcl("Content-Length: ");
        jcl.append(szLen);

        partlist = curl_slist_append(partlist, jcl.c_str());
        partlist = curl_slist_append(partlist, "Content-Type: application/vnd.tent.v0+json");
        partlist = curl_slist_append(partlist, "Content-Transfer-Encoding: binary");

        curl_formadd( &formpost,
                      &lastptr,
                      CURLFORM_COPYNAME, "jsonbody",
                      CURLFORM_COPYCONTENTS, szBody.c_str(),
                      CURLFORM_CONTENTHEADER, partlist,
                      CURLFORM_END);

        
        // Read in file 
        curl_slist *attachlist = 0;

        std::string cd;
        cd.append("Content-Disposition: form-data; name=\"attach\"; filename=\"");
        cd += szFileName;
        cd.append("\"");

        attachlist = curl_slist_append(attachlist, "Content-Transfer-Encoding: binary");
        //attachlist = curl_slist_append(attachlist, "Content-Disposition: form-data; name=\"buffer[0]\"; filename=\"thisthing.lst\"");
        attachlist = curl_slist_append(attachlist, cd.c_str());

        //
        char szBuffer[256];
        memset(szBuffer, '\0', sizeof(char)*256);                                              
        snprintf(szBuffer, (sizeof(char)*256),  "%d", uSize);     

        std::string cl("Content-Length: ");
        cl.append(szBuffer);

        std::cout<< "STRLEN : " << strlen(pData) << std::endl;
        std::cout<< "CL : " << cl <<std::endl;
        std::cout<< "USIZE : " << uSize << std::endl;
            
        attachlist = curl_slist_append(attachlist,  cl.c_str());
        attachlist = curl_slist_append(attachlist, "Content-Type: application/octet-stream");

        if(!pData)
        {
            std::cout<<"INVALID DATA PTR"<<std::endl;
            return;
        }

        curl_formadd( &formpost,
                      &lastptr,
                      CURLFORM_COPYNAME, "attatchment",
                      CURLFORM_PTRCONTENTS, pData,
                      CURLFORM_CONTENTSLENGTH, uSize,
                      CURLFORM_CONTENTHEADER, attachlist,
                      CURLFORM_END);


        //tdata* s = CreateDataObject();
        
        pCurl = curl_easy_init();
        multi_handle = curl_multi_init();
       
        /* initalize custom header list (stating that Expect: 100-continue is not
        *      wanted */ 
        headerlist = curl_slist_append(headerlist, buf);

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
            BuildAuthHeader(urlPath, std::string("POST"), szMacID, szMacKey, authheader);
            headerlist = curl_slist_append(headerlist, authheader.c_str());

            /* what URL that receives this POST */ 
            curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());

            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
            curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);

            curl_multi_add_handle(multi_handle, pCurl);

            // Set read response func and data
            //curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc); 
            //curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, s); 

            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutToString);
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseOut); 
        
            std::cout<<"here"<<std::endl;
            curl_multi_perform(multi_handle, &still_running);

            std::cout<<"here"<<std::endl;
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
            std::cout<<"here"<<std::endl;
            std::cout<<"finished..."<<std::endl;
            //responseOut.clear();
            //responseOut.append(ExtractDataToString(s));
            //DestroyDataObject(s);
            curl_multi_cleanup(multi_handle);

            /* then cleanup the formpost chain */ 
            curl_formfree(formpost);
                                                               
            /* free slist */ 
            curl_slist_free_all (headerlist);
            curl_slist_free_all (partlist);
            curl_slist_free_all (attachlist);
        }

    curl_easy_cleanup(pCurl);
}



void ConnectionManager::HttpPostWithAuth( const std::string &url, 
                                          const UrlParams* pParams,
                                          const std::string &body, 
                                          std::string &responseOut, 
                                          const std::string &szMacAlgorithm, 
                                          const std::string &szMacID, 
                                          const std::string &szMacKey, 
                                          bool verbose)
{

    CURL* pCurl = curl_easy_init();
    WriteOut postd; // Post content to be read
    postd.readptr = body.c_str(); // serialized json (should be)
    postd.sizeleft = body.size();

    CURLcode res; 
    tdata* s = CreateDataObject();

    if(verbose)
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);   

    std::string urlPath = url;
    EncodeAndAppendUrlParams(pCurl, pParams, urlPath);

    curl_slist *headers = 0; // Init to null, always
    headers = curl_slist_append(headers, "Accept: application/vnd.tent.v0+json" );
    headers = curl_slist_append(headers, "Content-Type: application/vnd.tent.v0+json");

    std::string authheader;
    BuildAuthHeader(urlPath, std::string("POST"), szMacID, szMacKey, authheader);
    headers = curl_slist_append(headers, authheader.c_str());

    // Set url
    curl_easy_setopt(pCurl, CURLOPT_URL, urlPath.c_str());
    // Set that we want to Post
    curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
    // Set the read function
    curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, read_callback);

    // Set Post data 
    curl_easy_setopt(pCurl, CURLOPT_READDATA, &postd);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, postd.sizeleft);

    // Write out headers 
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);

    // Set read response func and data
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteOutFunc); 
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, s); 
    
    res = curl_easy_perform(pCurl);

    if(res != CURLE_OK)
    {
        std::cout<<"Post failed... " << curl_easy_strerror(res) << std::endl;
    }

    responseOut.clear();
    responseOut.append(ExtractDataToString(s));
    DestroyDataObject(s);

    curl_easy_cleanup(pCurl);

}

void ConnectionManager::BuildAuthHeader( const std::string &url, 
                                         const std::string &requestMethod, 
                                         const std::string &szMacID, 
                                         const std::string &szMacKey, 
                                         std::string& out)
{
    std::string n;
    GenerateNonce(n);

    out.clear();
    out.append("Authorization: ");
    
    out.append("MAC id=\"");
    out.append(szMacID.c_str());
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
    SignRequest(requestString,szMacKey, signedreq);

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
void ConnectionManager::SignRequest(const std::string &szRequest, const std::string &szKey, std::string &out)
{
    std::string mac, encoded, som;

    try
    {
  //      std::cout<< "Key : " << szKey << std::endl;
        unsigned char szReqBuffer[szRequest.size()];
        memcpy(szReqBuffer, szKey.c_str(), strlen(szKey.c_str())+1);

 //       std::cout<< " BUFFER : " << szReqBuffer << std::endl;
        //CryptoPP::HMAC< CryptoPP::SHA256 > hmac(szReqBuffer, szRequest.size());

        CryptoPP::HMAC< CryptoPP::SHA256 > hmac(szReqBuffer, strlen(szKey.c_str())+1);

        CryptoPP::StringSource( szRequest,
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

static size_t WriteOutFunc(void *ptr, size_t size, size_t nmemb, struct tdata *s)
{
    std::string test;
    test.append((char*)ptr, size*nmemb);

    std::cout<< "TESTING : " << test << std::endl;

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


