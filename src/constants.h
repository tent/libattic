#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#pragma once

namespace attic { namespace cnst {
// Types
static const char* g_app_type = "https://tent.io/types/app/v0";
static const char* g_attic_cred_type = "https://attic.is/types/cred/v0";
static const char* g_attic_folder_type = "https://attic.is/types/folder/v0";
static const char* g_attic_file_type = "https://attic.is/types/file/v0";
static const char* g_attic_chunk_type = "https://attic.is/types/chunk/v0";
static const char* g_basic_profile_type = "https://tent.io/types/basic-profile/v0";
// Fragments
static const char* g_transit_fragment = "in_transit";
static const char* g_deleted_fragment = "deleted";
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
static const char* g_szWorkingPlaceHolder = "<working>";
// File splitting
static const unsigned int g_unSplitMin = 1 << 21; // ~2 mbs
static const unsigned int g_unSplitMax = 1 << 23; // ~8 mbs 
static const unsigned int g_unMaxBuffer = 1 << 26; // ~67 mbs <-- this needs to be at least 2mbs
static const unsigned int g_max_chunk_cluster = 30; // 30 chunks per cluster
// Config values
static const char* g_szConfigTrashPath = "trash_path";      // filepath to trash or recycling bin
static const char* g_szConfigUploadLimit = "upload_limit";  // upload limit in mbs
static const char* g_szConfigWorkingDir = "working_dir";    // filepath to working directory
static const char* g_szConfigTempDir = "temp_dir";          // filepath to temproray directory
static const char* g_szConfigConfigDir = "config_dir";      // filepath to config directory
static const char* g_szConfigEntityURL = "entity_url";      // entity url
// Link rel Types
static const char* g_meta_rel = "https://tent.io/rels/meta-post"; // Meta post rel
static const char* g_cred_rel = "https://tent.io/rels/credentials"; // Credentials rel
// Headers
static const char* g_accept_header = "application/vnd.tent.post.v0+json";
static const char* g_accept_octet_header = "application/octet-stream";
static const char* g_content_type_header = "application/vnd.tent.post.v0+json";
static const char* g_children_header = "application/vnd.tent.post-children.v0+json";
static const char* g_token_type = "https://tent.io/oauth/hawk-token"; // token type
// table names
static const char* g_filetable = "filetable";
static const char* g_foldertable = "foldertable";
}} //namespace
#endif

