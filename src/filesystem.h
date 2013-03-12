#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#pragma once

#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>

#include "errorcodes.h"

namespace fs
{
    static boost::filesystem::path MakePathRelative( boost::filesystem::path a_From, 
                                                     boost::filesystem::path a_To );

    static void MakePathRelative( const std::string& rootPath, 
                                  const std::string& secondPath, 
                                  std::string& relativeOut);

    static int GetCanonicalPath(const std::string& path, std::string& out);
    static int GetParentPath(const std::string& path, std::string& out);
    static int CreateDirectory(const std::string& path);
    static void CreateDirectoryTree(const std::string& filepath);
    static bool CheckFileExists(const std::string& filepath);

    static boost::filesystem::path MakePathRelative( boost::filesystem::path a_From, 
                                                     boost::filesystem::path a_To )
    {
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

    static int GetParentPath(const std::string& path, std::string& out) {
        int status = ret::A_OK;

        std::string ppath;
        status = GetCanonicalPath(path, ppath);
        if(status == ret::A_OK) {
            boost::filesystem::path p(ppath);
            boost::filesystem::path dir = p.parent_path();
            out = dir.string();
        }

        return status;
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

    static void CreateDirectoryTree(const std::string& filepath) {
        // Pass in full filepath /root/directory/some/other/file.txt
        try {
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

    static bool CheckFileExists(const std::string& filepath) {
        if(boost::filesystem::exists(filepath))
            return true;
        return false;
    }

    static void MoveFile(const std::string& originalpath, const std::string& newpath) {
        boost::filesystem::path from_fp(originalpath);
        CreateDirectoryTree(newpath);
        boost::filesystem::path to_fp(newpath);
        boost::filesystem::rename(from_fp, to_fp);
    }
};

#endif

