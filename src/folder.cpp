#include "folder.h"

#include "constants.h"

void FolderEntry::Serialize(Json::Value& root)
{
    root["postid"] = m_Postid;
    root["type"] = m_Type;
    root["path"] = m_Path;
}

void FolderEntry::Deserialize(Json::Value& root)
{
    m_Postid = root.get("postid", "").asString();
    m_Type = root.get("type", "").asString();
    m_Path = root.get("path", "").asString();
}


Folder::Folder()
{
    SetType(cnst::g_FolderType);
}

Folder::~Folder()
{
}

void Folder::Serialize(Json::Value& root)
{
    EntryList::iterator itr = m_Entries.begin();

    std::vector<std::string> serializedList;
    std::string val;
    for(;itr!=m_Entries.end(); itr++) {
        val.clear();
        jsn::SerializeObject(&(*itr), val);
        serializedList.push_back(val);
    }

    std::string sval;
    Json::Value folderval;
    jsn::SerializeVector(folderval, serializedList);
    jsn::SerializeJsonValue(folderval, sval);

    root["foldercontents"] = sval;

    FolderEntry::Serialize(root);
}

void Folder::Deserialize(Json::Value& root)
{
    FolderEntry::Deserialize(root);

    std::string sval;
    Json::Value folderval;
                                              
    std::vector<std::string> serializedList;                              

    sval = root.get("foldercontents", "").asString();

    jsn::DeserializeJsonValue(folderval, sval);                            
    jsn::DeserializeIntoVector(folderval, serializedList);                 

    if(serializedList.size() > 0) {
        std::vector<std::string>::iterator itr = serializedList.begin();  
        for(;itr != serializedList.end(); itr++) {
            FolderEntry fe;
            jsn::DeserializeObject(&fe, (*itr));                          
            m_Entries.push_back(fe); // copy                            
        }                                                                 
    }                                                                     
}


