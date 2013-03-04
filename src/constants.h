

#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#pragma once

namespace cnst
{
    // Profile Types
    static const char* g_szBasicProfileType = "https://tent.io/types/info/basic/v0.1.0";
    static const char* g_szCoreProfileType = "https://tent.io/types/info/core/v0.1.0";
    static const char* g_szAtticProfileType = "https://cupcake.io/types/info/attic/v0.1.0";

    // Link Header profile relationship 
    static const char* g_szProfileRel = "https://tent.io/rels/profile";

    // Attic Post Types
    static const char* g_szFileMetadataPostType = "https://cupcake.io/types/post/attic-file/v0.1.0";
    static const char* g_szChunkStorePostType = "https://cupcake.io/types/post/attic-chunks/v0.1.0";
    static const char* g_szFolderPostType = "https://cupcake.io/types/post/attic-folder/v0.1.0";

    // TODO :: Remove, Depricated
    static const char* g_szAtticPostType = "https://tent.io/types/post/attic/v0.1.0";    
    static const char* g_szAtticMetaStorePostType = "https://tent.io/types/post/attic-metastore/v0.1.0";

    // Filenames
    static const char* g_szAppDataName = "app";                                              
    static const char* g_szAuthTokenName = "at";                                             
    static const char* g_szPhraseTokenName = "phst";
    static const char* g_szManifestName = "manifest";                                        
    static const char* g_szEntityName = "ent";

    // Chunk Data
    static unsigned int g_unChunkSize = 400000; // just default arbitrary number, to be overriden

    // Folder Entry Types
    static const char* g_szFolderType = "folder";
    static const char* g_szFileType = "file";

    // File splitting
    static const unsigned int g_uSplitMin = 1 << 21; // ~2 mbs
    static const unsigned int g_unSplitMax = 1 << 23; // ~8 mbs 
    static const unsigned int g_unMaxBuffer = 1 << 24; // ~16 mbs
};


#endif

