#ifndef CLIENTUTILS_H_
#define CLIENTUTILS_H_ 
#pragma once

#include <string>

#include "utils.h"
#include "netlib.h"
#include "constants.h"
#include "entity.h"
#include "entitypost.h"
#include "accesstoken.h"
#include "envelope.h"
#include "posthandler.h"

namespace attic { namespace client {

static int HeadRequestEntity(const std::string& entityurl, std::string& linkOut);
static void ExtractMetaLink(Response& response, std::string& linkOut);
static void ExtractLink(const std::string& link, std::string& out);

static void ConstructMetaPath(const std::string& entityurl,
                              const std::string& metaendpoint,
                              std::string& out);

static int Discover(const std::string& entityurl, const AccessToken* at, Entity& entOut) {
    int status = ret::A_OK;
    std::string meta_link;
    std::cout<<" head request : " << entityurl << std::endl;
    status = HeadRequestEntity(entityurl, meta_link);
    std::cout<<" META LINK : " << meta_link << std::endl;
    if(status == ret::A_OK) {
        PostHandler<EntityPost> ph;
        EntityPost ep;
        status = ph.Get(meta_link, NULL, ep);

        std::cout<<" CODE : " << ph.response().code << std::endl;
        std::cout<<" BODY : " << ph.response().body << std::endl;
        if(status == ret::A_OK) {
            entOut = ep.entity();
            Json::Value val;
            jsn::SerializeObject(&entOut, val);
            jsn::PrintOutJsonValue(&val);
        }
        else {
            std::cout<<" FAIL DISCOVERY NON 200 " << std::endl;
            status = ret::A_FAIL_NON_200;
        }
    }
    std::cout<<" Discover status : " << status << std::endl;
    return status; 
}

static int HeadRequestEntity(const std::string& entityurl, std::string& linkOut) {
    int status = ret::A_OK;

    Response response;
    netlib::HttpHead(entityurl, NULL, NULL, response);
    std::cout<<" HEAD CODE : " << response.code << std::endl;
    std::cout<<" HEAD BODY : " << response.body << std::endl;
    std::cout<<" headers : " << response.header.asString() << std::endl;

    if(response.code == 200) {
        std::string link;
        ExtractMetaLink(response, link);
        std::string metapath;
        ConstructMetaPath(entityurl, link, metapath);
        linkOut = metapath;
    }
    else { 
        std::cout<<" HEAD REQUST FAIL NON 200 " << std::endl;
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

static void ConstructMetaPath(const std::string& entityurl,
                              const std::string& metaendpoint,
                              std::string& out) {
    std::cout<<" entity url : " << entityurl << std::endl;
    std::cout<<" meta endpoint : " << metaendpoint << std::endl;
    // uri decode
    std::string endpoint = netlib::UriDecode(metaendpoint);
    std::cout<<" meta endpoint (decoded) : " << endpoint << std::endl;
    // Check if already absolute
    //
    size_t ent_pos = endpoint.find(entityurl);
    if(ent_pos != std::string::npos && ent_pos != 0) {
        // Construct meta path
        out = entityurl;
        utils::CheckUrlAndRemoveTrailingSlash(out);
        out += endpoint;
    }
    else {
        out = endpoint;
    }
}

static void ExtractMetaLink(Response& response, std::string& linkOut) {
    std::string link_header = response.header["Link"];
    if(!link_header.empty()){
        if(link_header.find(cnst::g_meta_rel) != std::string::npos)
            ExtractLink(link_header, linkOut);
    }
}

static void ExtractLink(const std::string& link, std::string& out) {
    size_t begin = link.find("<");
    size_t end = link.find(">");
    size_t diff = (end - (begin+1));
    out = link.substr(begin+1, diff);
}






}} //namespace
#endif

