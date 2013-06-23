#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#pragma once

#include <string>
#include <iostream>
#include <vector>

#include <stdio.h>

#include <boost/filesystem.hpp>
#include <boost/version.hpp>

#include "errorcodes.h"

namespace attic { namespace fs {
static boost::filesystem::path MakePathRelative( boost::filesystem::path a_From, 
                                                 boost::filesystem::path a_To );

static void MakePathRelative( const std::string& rootPath, 
                              const std::string& secondPath, 
                              std::string& relativeOut);

static int GetCanonicalPath(const std::string& path, std::string& out);
static bool GetParentPath(const std::string& path, std::string& out);
static int CreateDirectory(const std::string& path);
static void CreateDirectoryTree(const std::string& filepath);
static void CreateDirectoryTreeForFolder(const std::string& folderpath);
static bool CheckFilepathExists(const std::string& filepath);
static bool DeleteFile(const std::string& filepath);
static void ScanDirectory(const std::string& folderpath, std::vector<std::string>& paths_out);

static void ExtractSubDirectories(const std::string& root, 
                                  const std::string& filepath, 
                                  std::vector<std::string> out);


static boost::filesystem::path MakePathRelative( boost::filesystem::path a_From, 
                                                 boost::filesystem::path a_To ) {
    a_From = boost::filesystem::absolute( a_From ); a_To = boost::filesystem::absolute( a_To );
    boost::filesystem::path ret;
    boost::filesystem::path::const_iterator itrFrom( a_From.begin() ), itrTo( a_To.begin() );
    // Find common base
    for( boost::filesystem::path::const_iterator toEnd( a_To.end() ), fromEnd( a_From.end() ) ; itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo );
    // Navigate backwards in directory to reach previously found base
    for( boost::filesystem::path::const_iterator fromEnd( a_From.end() ); itrFrom != fromEnd; ++itrFrom ) {
        if( (*itrFrom) != "." )
        ret /= "..";
    }
    // Now navigate down the directory branch
    for(;itrTo != a_To.end(); ++itrTo)
        ret /= *itrTo;

    return ret;
}

static void MakePathRelative( const std::string& rootPath, 
                              const std::string& secondPath, 
                              std::string& relativeOut)
{
    boost::filesystem::path root(rootPath.c_str());
    boost::filesystem::path second(secondPath.c_str());

    relativeOut = MakePathRelative( root, second ).string();
}

static int GetCanonicalPath(const std::string& path, std::string& out) {
    int status = ret::A_OK;
    boost::filesystem::path root(path.c_str());

    if(boost::filesystem::exists(root)){
        boost::system::error_code error;
        boost::filesystem::path can = boost::filesystem::canonical(root, error);

        if(!error) {
            out = can.string();
        }
        else {
            std::cout<< " In GetCanonicalPath ... " << std::endl;
            std::cout<< boost::system::system_error(error).what() << std::endl;
            status = ret::A_FAIL_FS_ERROR;
        }
    }
    else {
        status = ret::A_FAIL_PATH_DOESNT_EXIST;
    }

    return status;
}

static bool GetParentPath(const std::string& path, std::string& out) {
    bool ret = false;

    std::string ppath;
    int status = GetCanonicalPath(path, ppath);
    if(status == ret::A_OK) {
        boost::filesystem::path p(ppath);
        boost::filesystem::path dir = p.parent_path();
        out = dir.string();
        ret = true;
    }

    return ret;
}

static int CreateDirectory(const std::string& path) {
    std::cout<<" creating directory ... " << std::endl;
    int status = ret::A_OK;

    boost::filesystem::path root(path);

    if(!boost::filesystem::exists(root)){
        std::cout<<"doesn't exist ... " << std::endl;
        if (!boost::filesystem::create_directory(root)) { 
            std::cout<<"failed to create" << std::endl;
            status = ret::A_FAIL_CREATE_DIRECTORY;
        }
    }

    return status;
}

// Pass in full folderpath, then popolate the deque with all parent directories
static void CreateDirectoryTree(const std::string& folderpath, 
                                const std::string working_dir, 
                                std::deque<std::string>& out) {
    try {
        std::cout<<" incoming path : " << folderpath << std::endl;
        boost::filesystem::path root(folderpath);
        boost::filesystem::path working(working_dir);

        if(!boost::filesystem::exists(root)) {
            std::cout<<" folder does not exist : " << root.string() << std::endl;
            boost::filesystem::create_directories(root);
        }

        boost::filesystem::path parent = root;
        while(parent != working) {
            std::cout<<" pushing back : " << parent.string() << std::endl;
            out.push_back(parent.string());
            parent = parent.parent_path();
        }
        out.push_back(parent.string());
    }
    catch(boost::filesystem::filesystem_error& er) {
        std::cout<<" error : " << er.what() << std::endl; 
    }
}

static void CreateDirectoryTreeForFolder(const std::string& folderpath) {
    try {
        std::cout<<" incoming path : " << folderpath << std::endl;
        boost::filesystem::path root(folderpath);
        if(!boost::filesystem::exists(root)) {
            std::cout<<" creating path : " << root << std::endl;
            boost::filesystem::create_directories(root);
        }
    }
    catch(boost::filesystem::filesystem_error& er) {
        std::cout<<" error : " << er.what() << std::endl; 
    }
}
static void CreateDirectoryTree(const std::string& filepath) {
    // Pass in full filepath /root/directory/some/other/file.txt
    try {
        std::cout<<" incoming path : " << filepath << std::endl;
        boost::filesystem::path root(filepath);
        boost::filesystem::path parent = root.parent_path();
        
        if(!boost::filesystem::exists(root)) {
            std::cout<<" folder does not exist : " << parent.string() << std::endl;
            boost::filesystem::create_directories(parent);
        }
    }
    catch(boost::filesystem::filesystem_error& er) {
        std::cout<<" error : " << er.what() << std::endl; 
    }
}

static bool CheckFilepathExists(const std::string& filepath) {
    if(boost::filesystem::exists(filepath))
        return true;
    return false;
}

static void MoveFileToFolder(const std::string& filepath, const std::string& folderpath) {
    std::string filename;
    size_t pos = filepath.rfind("/");
    if(pos != std::string::npos) {
        filename = filepath.substr(pos);

        std::string new_path = folderpath;
        if(new_path[new_path.size()-1] != '/')
            new_path.append("/");
        new_path.append(filename);

        boost::filesystem::path from(filepath);
        boost::filesystem::path to(new_path);
        for(int i=0; boost::filesystem::exists(to)!=false; i++) {
            std::string suf = "(";
            char buf[256] = {'\0'};
            snprintf(buf, 256, "%d", i);
            suf += buf; 
            suf += ")";
            to += suf;
        }
        boost::filesystem::rename(from, to);
    }
}

static void MoveFile(const std::string& originalpath, const std::string& newpath) {
    boost::filesystem::path from_fp(originalpath);
    CreateDirectoryTree(newpath);
    boost::filesystem::path to_fp(newpath);
    boost::filesystem::rename(from_fp, to_fp);
}

static bool DeleteFile(const std::string& filepath) { 
    if(CheckFilepathExists(filepath)) {
        boost::filesystem::path root(filepath);
        boost::system::error_code ec;
        boost::filesystem::remove(root, ec);
        if(ec) {
            boost::system::system_error error(ec);
            std::cout<<" FILESYSTEM ERROR : " << error.what() << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

static void ScanDirectory(const std::string& folderpath, std::vector<std::string>& paths_out) {
    boost::filesystem::path root(folderpath);
    if(boost::filesystem::exists(root)){
        boost::filesystem::directory_iterator itr(root);
        for(;itr != boost::filesystem::directory_iterator(); itr++ ){
            if(itr->status().type() == boost::filesystem::regular_file){
                paths_out.push_back(itr->path().string());
            }
            else if(itr->status().type() == boost::filesystem::directory_file) {
                ScanDirectory(itr->path().string(), paths_out);
            }
        }
    }
}

static void ExtractSubDirectories(const std::string& root, 
                                  const std::string& filepath, 
                                  std::vector<std::string> out) {

    // Assuming these are already canonical paths
    // Make sure the two paths are actually related
    if(filepath.find(root) != std::string::npos) {
        boost::filesystem::path root_path(root);
        boost::filesystem::path file_path(filepath);

        while(file_path.parent_path() != root_path) {
            out.push_back(file_path.parent_path().string());
            file_path = file_path.parent_path();
        }
    }
}

static bool RenamePath(const std::string& original_path, const std::string& new_path) {
    bool ret = true;
    boost::filesystem::path original(original_path), newpath(new_path);
    //boost::filesystem::rename(original, newpath);
    
    std::cout<<" RENAMING : " << original_path << " TO : " << new_path << std::endl;
    int status = rename(original_path.c_str(), new_path.c_str());
    if(status != 0) { 
        std::cout<<" RenamePath failed status : " << status << std::endl;
        ret = false;
    }
    std::cout <<" RENAME STATUS : " << status << std::endl;
    return ret;
}


static void ErrorCheckPathDoubleQuotes(std::string& path) {
    size_t pos = path.find("//");
    while(pos != std::string::npos) {
        path.replace(pos, 2, "/");
        pos = path.find("//");
    }
}

}}//namespace
#endif

