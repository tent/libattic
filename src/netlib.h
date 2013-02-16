#ifndef NETLIB_H_
#define NETLIB_H_
#pragma once

#define BOOST_NETWORK_ENABLE_HTTPS 

#include <boost/network/protocol/http/client.hpp>
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
 
#include <hex.h>        // cryptopp
#include <hmac.h>       // cryptopp
#include <base64.h>     // cryptopp
 
#include "url.h"
#include "urlparams.h"
#include "errorcodes.h"
#include "accesstoken.h"
#include "connectionmanager.h"

using namespace boost::network;

namespace netlib
{
    // Forward Declarations ******************************************************
    static int HttpGet( const std::string& url, 
                        const AccessToken* at, 
                        Response& out);

    static int HttpPost( const std::string& url, 
                         const AccessToken* at, 
                         Response& out);

    static void GenerateHmacSha256(std::string &out);

    static void BuildRequest( const std::string& url,
                              const std::string& requestMethod,
                              const AccessToken* at,
                              http::client::request& reqOut);

    static void BuildAuthHeader( const std::string &url, 
                                 const std::string &requestMethod, 
                                 const std::string &macid, 
                                 const std::string &mackey, 
                                 std::string& out);
 
    static void GenerateNonce(std::string &out);

    static void SignRequest( const std::string &request, 
                             const std::string &key, 
                             std::string &out);
 
    static void GenerateHmacSha256(std::string &out);

    // Definitions start ***********************************************************
    static int HttpGet(const std::string& url, const AccessToken* at, Response& out)
    {
        int sstatus = ret::A_OK;

        http::client::request request;
        BuildRequest( url,
                      "GET",
                      at,
                      request);

        http::client client;
        http::client::response response = client.get(request);

        int st = status(response);
        std::cout << " STATUS : " << status(response) << std::endl;
        std::cout << " BODY : " << body(response) << std::endl;

        out.code = status(response);
        out.body = body(response);
    
        return sstatus;
    }

    static int HttpPost( const std::string& url, 
                         const AccessToken* at, 
                         Response& out)
    {
        int sstatus = ret::A_OK;

        http::client::request request;
        BuildRequest( url,
                      "POST",
                      at,
                      request);

        http::client client;
        http::client::response response = client.post(request);

        int st = status(response);
        std::cout << " STATUS : " << status(response) << std::endl;
        std::cout << " BODY : " << body(response) << std::endl;

        out.code = status(response);
        out.body = body(response);

        return sstatus;
    }


    // Utility Functions ***********************************************************
    static void BuildRequest( const std::string& url,
                              const std::string& requestMethod,
                              const AccessToken* at,
                              http::client::request& reqOut)
    {
        reqOut.uri(url);
        std::string authheader;
        if(at)
        {
            // Build Auth Header
            BuildAuthHeader( url,
                             requestMethod,
                             at->GetAccessToken(),
                             at->GetMacKey(),
                             authheader );

        }

        reqOut << header("Connection", "close");
        reqOut << header("Accept:", "application/vnd.tent.v0+json" );
        reqOut << header("Content-Type:", "application/vnd.tent.v0+json");

        if(!authheader.empty())
            reqOut << header("Authorization: " , authheader);
    }

    static void BuildAuthHeader( const std::string &url, 
                                 const std::string &requestMethod, 
                                 const std::string &macid, 
                                 const std::string &mackey, 
                                 std::string& out)
    {
        std::string n;
        GenerateNonce(n);

        out.clear();
        //out.append("Authorization: ");
        
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
        

       // //std::cout << "REQUEST_STRING : " << requestString << std::endl;
       // //std::cout << "AUTH_HEADER : " << out << std::endl;
    }

    static void GenerateNonce(std::string &out)
    {
        out.clear();
        std::string seed;
        for(int i=0; i<3; i++)
            seed+=utils::GenerateChar();

        utils::StringToHex(seed, out);
    }

   static void SignRequest( const std::string &request, 
                             const std::string &key, 
                             std::string &out)
    {
        std::string mac, encoded, som;

        try
        {
      //      //std::cout<< "Key : " << key << std::endl;
            unsigned char szReqBuffer[request.size()];
            memcpy(szReqBuffer, key.c_str(), strlen(key.c_str())+1);

     //       //std::cout<< " BUFFER : " << szReqBuffer << std::endl;
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


    //    //std::cout << "hmac: " << encoded << std::endl; 
    //    //std::cout << "mac : " << mac << std::endl;

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

    static void GenerateHmacSha256(std::string &out)
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

        //std::cout << "key: " << encoded << std::endl;
        //std::cout << "plain text: " << plain << std::endl;

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

        //std::cout << "hmac: " << encoded << std::endl; 

        out.clear();
        out = encoded;
    }

};

#endif

