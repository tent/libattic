

#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#pragma once

namespace cnst
{
    // Link Header profile relationship 
    static const char* g_szProfileRel = "https://tent.io/rels/profile";

    // Attic Post Types
    static const char* g_szFileMetadataPostType = "https://cupcake.io/types/post/attic-file/v0.1.0";
    static const char* g_szChunkStorePostType = "https://cupcake.io/types/post/attic-chunks/v0.1.0";
    static const char* g_szFolderPostType = "https://cupcake.io/types/post/attic-folder/v0.1.0";

    // TODO :: Remove, Depricated
    static const char* g_szAtticPostType = "https://tent.io/types/post/attic/v0.1.0";    
    static const char* g_szAtticMetaStorePostType = "https://tent.io/types/post/attic-metastore/v0.1.0";

    static const char* g_szAppDataName = "app";                                              
    static const char* g_szAuthTokenName = "at";                                             
    static const char* g_szPhraseTokenName = "phst";
    static const char* g_szManifestName = "manifest";                                        
};


#endif

