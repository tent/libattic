

#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#pragma once

namespace cnst
{
    // Post Types
    static const char* g_szFileMetadataPostType = "https://cupcake.io/types/post/attic-file/v0.1.0";
    static const char* g_szChunkStorePostType = "https://cupcake.io/types/post/attic-chunks/v0.1.0";
    static const char* g_szFolderPostType = "https://cupcake.io/types/post/attic-folder/v0.1.0";

    // TODO :: Remove, Depricated
    static const char* g_szAtticPostType = "https://tent.io/types/post/attic/v0.1.0";    
    static const char* g_szAtticMetaStorePostType = "https://tent.io/types/post/attic-metastore/v0.1.0";


    static const char* g_szAppData = "app";                                              
    static const char* g_szAuthToken = "at";                                             
    static const char* g_szManifest = "manifest";                                        
};


#endif

