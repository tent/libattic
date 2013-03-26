#include "post.h"

#include <stdio.h>
#include "tentapp.h"

Post::Post() {
    m_TentApp = 0;
    m_PublishedAt = 0;
    m_ReceivedAt = 0;
}

Post::~Post() {
    if(m_Attachments.size() > 0) {
        std::cout<<" # attachments : " << m_Attachments.size() << std::endl;

        AttachmentVec::iterator itr = m_Attachments.begin();

        Attachment* pAtch=0;
        for(;itr != m_Attachments.end();) {
            //pAtch = *itr;
            itr++;
            /*
            if(pAtch)
            {
                std::cout<<"Name : " << pAtch->Name << std::endl;
                std::cout<<"deleting ... " << std::endl;
                delete pAtch;
                pAtch = NULL;
            }
            */
        }

        m_Attachments.clear();
        std::cout<<" # attachments : " << m_Attachments.size() << std::endl;
    }
}

void Post::GetContent(const std::string& key, Json::Value& out) {
    ContentMap::iterator itr = m_Content.find(key);
    if(itr != m_Content.end())
        out = itr->second;
}

void Post::Serialize(Json::Value& root) {
    // General Post
    if(!m_ID.empty())
        root["id"] = m_ID;
    if(!m_Entity.empty())
        root["entity"] = m_Entity;

    if(m_PublishedAt > 0)
        root["published_at"] = m_PublishedAt;

    if(m_Mentions.size() > 0) {
        Json::Value mentions;
        jsn::SerializeVector(m_Mentions, mentions);
        root["mentions"] = mentions;
    }
    
    if(m_Licenses.size() > 0) {
        Json::Value licenses;
        jsn::SerializeVector(m_Licenses, licenses);
        root["licenses"] = licenses;
    }

    if(!m_Type.empty())
        root["type"] = m_Type;
   
    if(m_Content.size() > 0) {
        // TODO::this
        Json::Value content(Json::objectValue); // We want scopes to be an object {}// vs []
        jsn::SerializeMapIntoObject(content, m_Content);
        root["content"] = content;
    }

    if(m_Attachments.size() > 0) {
        // TODO::this
    }

    if(m_TentApp) {
        Json::Value app;
        m_TentApp->Serialize(app); 
        root["app"] = app;
    }

    if(m_Views.size() > 0) {
        Json::Value views(Json::objectValue); // We want scopes to be an object {}// vs []
        jsn::SerializeMapIntoObject(views, m_Views);
        root["views"] = views;
    }

    Json::Value permissions(Json::objectValue); // We want scopes to be an object {}// vs []
    jsn::SerializeObject(&m_Permissions, permissions);
    root["permissions"] = permissions;

    //root["version"] = m_Version;
}

void Post::Deserialize(Json::Value& root) {
    // General Post
    m_ID            = root.get("id", "").asString();
    m_Entity        = root.get("entity", "").asString();
    std::string pub = root.get("published_at", "").asString();
    m_PublishedAt = atoi(pub.c_str());
    std::string rec = root.get("received_at", "").asString();
    m_ReceivedAt = atoi(rec.c_str());

    jsn::DeserializeObject(&version_, root["version"]);

    jsn::DeserializeIntoVector(root["mentions"], m_Mentions);
    jsn::DeserializeIntoVector(root["licenses"], m_Licenses);

    // TODO :: content is dynamic, and can be a variety of things
    //         serialization and deserialization is more complex than
    //         just a map of strings
   
    jsn::DeserializeObjectValueIntoMap(root["content"], m_Content);
    
    // Deserialize this into an array of objects
    Json::Value atch(Json::arrayValue);
    atch = root["attachments"];

    if(atch.size() > 0) {
        Json::ValueIterator itr = atch.begin();           

        for(; itr != atch.end(); itr++) {
            //Attachment* pAtch = new Attachment;
            Attachment pAtch;
            Json::Value aobj(Json::objectValue);
            aobj = (*itr);

            if(aobj.isObject()) {
                if(aobj.size() >= 4) {
                    Json::ValueIterator ii = aobj.begin();

                    for(; ii != aobj.end(); ii++) {
                        pAtch.AssignKeyValue(ii.key().asString(), (*ii));
                    }
                }
            }

            m_Attachments.push_back(pAtch);
        }

    }


    if(!root["app"].isNull()) {
        //m_TentApp = new TentApp();
        // TODO :: this
        //m_TentApp->Deserialize(root["app"].asObject());
    }

    jsn::DeserializeObjectValueIntoMap(root["views"], m_Views);
    jsn::DeserializeObject(&m_Permissions,root["permissions"]);
}


