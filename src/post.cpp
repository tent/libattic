#include "post.h"

#include "tentapp.h"

Post::Post()
{
    m_TentApp = 0;
    m_PublishedAt = 0;
    m_ReceivedAt = 0;
}

Post::~Post()
{
    if(m_Attachments.size() > 0)
    {

        AttachmentVec::iterator itr = m_Attachments.begin();

        Attachment* pAtch=0;
        for(;itr != m_Attachments.end();)
        {
            pAtch = *itr;
            itr++;
            delete pAtch;
            pAtch = 0;
        }

        m_Attachments.clear();

    }

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
   
    if(m_Content.size() > 0)
    {
        // TODO::this
        Json::Value content(Json::objectValue); // We want scopes to be an object {}// vs []
        JsonSerializer::SerializeMapIntoObject(content, m_Content);
        root["content"] = content;
    }

    if(m_Attachments.size() > 0)
    {
        // TODO::this
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

    root["version"] = m_Version;
}

void Post::Deserialize(Json::Value& root)
{
    m_ID = root.get("id", "").asString();
    m_Entity = root.get("entity", "").asString();
    m_PublishedAt = root.get("published_at", "").asInt(); 
    m_ReceivedAt = root.get("received_at", "").asInt();

    m_Version = root.get("version", "").asUInt();

    JsonSerializer::DeserializeIntoVector(root["mentions"], m_Mentions);
    JsonSerializer::DeserializeIntoVector(root["licenses"], m_Licenses);

    m_Type = root.get("type", "").asString();

    // TODO :: content is dynamic, and can be a variety of things
    //         serialization and deserialization is more complex than
    //         just a map of strings
    /*
    JsonSerializer::DeserializeObjectValueIntoMap(root["content"], m_Content);
    */

    // Deserialize this into an array of objects

    Json::Value atch(Json::arrayValue);

    atch = root["attachments"];

    if(atch.size() > 0)
    {
        std::cout<< " ARRAY INDEX : " << atch.size() << std::endl;

        Json::ValueIterator itr = atch.begin();           

        for(; itr != atch.end(); itr++)                   
        {                                                
            Attachment* pAtch = new Attachment;
            Json::Value aobj(Json::objectValue);
            aobj = (*itr);

            if(aobj.isObject())
            {

                if(aobj.size() == 4)
                {
                    Json::ValueIterator ii = aobj.begin();

                    for(; ii != aobj.end(); ii++)
                    {
                        std::cout<<ii.key().asString()<<std::endl;
                        std::cout<<*ii << std::endl;
                        pAtch->AssignKeyValue(ii.key().asString(), (*ii));
                    }
                    //pAtch->AssignKeyValue(itr.key(), *itr);
                }
            }

            //JsonSerializer::DeserializeObject(pAtch, aobj.asString());
            m_Attachments.push_back(pAtch);
        }
    }


    if(!root["app"].isNull())
    {
        //m_TentApp = new TentApp();
        // TODO :: this
        //m_TentApp->Deserialize(root["app"].asObject());
    }

    JsonSerializer::DeserializeObjectValueIntoMap(root["views"], m_Views);
    JsonSerializer::DeserializeObjectValueIntoMap(root["permissions"], m_Permissions);

}


