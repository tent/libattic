#include "post.h"

#include "tentapp.h"

Post::Post()
{
    m_PublishedAt = 0;
    m_ReceivedAt = 0;
}

Post::~Post()
{

}

void Post::Serialize(Json::Value& root)
{
    if(!m_ID.empty())
        root["id"] = m_ID;
    if(!m_Entity.empty())
        root["entity"] = m_Entity;
    root["published_at"] = m_PublishedAt;
    root["received_at"] = m_ReceivedAt;

    if(m_Mentions.size() > 0)
    {
        Json::Value mentions;
        JsonSerializer::SerializeVector(mentions, m_Mentions);
        root["mentions"] = mentions;
    }
    
    if(m_Licenses.size() > 0)
    {
        Json::Value licenses;
        JsonSerializer::SerializeVector(licenses, m_Licenses);
        root["licenses"] = licenses;
    }

    if(!m_Type.empty())
        root["type"] = m_Type;
   
    if(!m_Content.size() > 0)
    {
        // TODO::this
        Json::Value views(Json::objectValue); // We want scopes to be an object {}// vs []
        JsonSerializer::SerializeMapIntoObject(views, m_Content);
        root["content"] = views;
    }

    if(!m_Attachments.size() > 0)
    {
        Json::Value attachments;
        JsonSerializer::SerializeVector(attachments, m_Attachments);
        root["attachments"] = attachments;
    }

    if(m_TentApp)
    {
        Json::Value app;
        m_TentApp->Serialize(app); 
        root["app"] = app;

    }

    if(m_Views.size() > 0)
    {
        Json::Value views(Json::objectValue); // We want scopes to be an object {}// vs []
        JsonSerializer::SerializeMapIntoObject(views, m_Views);
        root["views"] = views;
    }

    if(m_Permissions.size() > 0)
    {
        Json::Value permissions(Json::objectValue); // We want scopes to be an object {}// vs []
        JsonSerializer::SerializeMapIntoObject(permissions, m_Permissions);
        root["permissions"] = permissions;
    }
}

void Post::Deserialize(Json::Value& root)
{
    m_ID = root.get("id", "").asString();
    m_Entity = root.get("entity", "").asString();
    m_PublishedAt = root.get("published_at", "").asInt(); 
    m_ReceivedAt = root.get("received_at", "").asInt();

    JsonSerializer::DeserializeIntoVector(root["mentions"], m_Mentions);
    JsonSerializer::DeserializeIntoVector(root["licenses"], m_Licenses);

    m_Type = root.get("type", "").asString();

    JsonSerializer::DeserializeObjectValueIntoMap(root["content"], m_Content);
    JsonSerializer::DeserializeIntoVector(root["attachments"], m_Attachments);


    if(!root["app"].isNull())
    {
        //m_TentApp = new TentApp();
        // TODO :: this
        //m_TentApp->Deserialize(root["app"].asObject());
    }

    JsonSerializer::DeserializeObjectValueIntoMap(root["views"], m_Views);
    JsonSerializer::DeserializeObjectValueIntoMap(root["permissions"], m_Permissions);

}


