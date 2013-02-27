#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>

namespace fs
{
    static boost::filesystem::path MakePathRelative( boost::filesystem::path a_From, boost::filesystem::path a_To );

    static void MakePathRelative( const std::string& rootPath, 
                                  const std::string& secondPath, 
                                  std::string& relativeOut);

    static boost::filesystem::path MakePathRelative( boost::filesystem::path a_From, boost::filesystem::path a_To )
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
    
};

#endif

