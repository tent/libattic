#include "folder.h"

#include "constants.h"

namespace attic { 

FolderEntry::FolderEntry() {}

FolderEntry::FolderEntry(const std::string& postid,
                         const std::string& type,
                         const std::string& path) {
    m_Postid = postid;
    m_Type = type;
    m_Path = path;
}

FolderEntry::~FolderEntry() {}

void FolderEntry::Serialize(Json::Value& root) {
    root["postid"] = m_Postid;
    root["type"] = m_Type;
    root["path"] = m_Path;
}

void FolderEntry::Deserialize(Json::Value& root) {
    m_Postid = root.get("postid", "").asString();
    m_Type = root.get("type", "").asString();
    m_Path = root.get("path", "").asString();
}


Folder::Folder() {
    SetType(cnst::g_szFolderType);
}

Folder::~Folder() {}

void Folder::Serialize(Json::Value& root) {
    /*
    std::string sval;
    SerializeContents(sval);
    root["foldercontents"] = sval;
    */

    Json::Value sval(Json::objectValue);
    SerializeContents(sval);
    root["foldercontents"] = sval;

    FolderEntry::Serialize(root);
}

void Folder::SerializeContents(Json::Value& root)
{
    EntryList::iterator itr = m_Entries.begin();

    for(;itr!=m_Entries.end(); itr++) {
        Json::Value val(Json::objectValue);
        jsn::SerializeObject(&(itr->second), val);
        root[itr->first] = val;
    }

}

void Folder::SerializeContents(std::string& out)
{
    EntryList::iterator itr = m_Entries.begin();

    std::vector<std::string> serializedList;
    std::string val;
    for(;itr!=m_Entries.end(); itr++) {
        val.clear();
        jsn::SerializeObject(&(itr->second), val);
        serializedList.push_back(val);
    }

    Json::Value folderval;
    jsn::SerializeVector(serializedList, folderval);
    jsn::SerializeJsonValue(folderval, out);
}

void Folder::Deserialize(Json::Value& root)
{
    FolderEntry::Deserialize(root);
    //std::string sval = root.get("foldercontents", "").asString();
    //DeserializeContents(sval);

    Json::Value sval = root["foldercontents"];
    DeserializeContents(sval);

}

void Folder::DeserializeContents(Json::Value& root)
{
    Json::ValueIterator itr = root.begin();

    for(; itr != root.end(); itr++) {
        FolderEntry fe;
        jsn::DeserializeObject(&fe, (*itr));
        m_Entries[itr.key().asString()] = fe;
    }
}

void Folder::DeserializeContents(const std::string& in)
{
    Json::Value folderval;
    std::vector<std::string> serializedList;                              

    jsn::DeserializeJsonValue(folderval, in);                            
    jsn::DeserializeIntoVector(folderval, serializedList);                 

    if(serializedList.size() > 0) {
        std::vector<std::string>::iterator itr = serializedList.begin();  
        for(;itr != serializedList.end(); itr++) {
            FolderEntry fe;
            jsn::DeserializeObject(&fe, (*itr));                          
            m_Entries[fe.GetPath()] = fe; // copy                            
        }                                                                 
    }     
}

bool Folder::GetFolderEntry(const std::string& path, FolderEntry& entry)
{
    EntryList::iterator itr = m_Entries.find(path);
    if( itr != m_Entries.end()) {
       entry = itr->second; 
       return true;
    }

    return false;
}

} //namespace
