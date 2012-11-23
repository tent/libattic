
#ifndef JSONSERIALIZABLE_H_
#define JSONSERIALIZABLE_H_
#pragma once

#include <string>

#include <json/json.h>

class JsonSerializable
{
public:
    JsonSerializable() {};
    virtual ~JsonSerializable() {};

    virtual void Serialize(Json::Value& root) = 0;
    virtual void Deserialize(Json::Value& root) = 0;
};

class JsonSerializer
{
    JsonSerializer(){}
    JsonSerializer(const JsonSerializer& rhs) {}
    JsonSerializer operator=(const JsonSerializer& rhs) { return *this; }
public:

    static bool SerializeObject(JsonSerializable* pObj, std::string& output)
    {
        if(!pObj)
            return false;

        Json::Value root;
        pObj->Serialize(root);

        Json::StyledWriter writer;
        output = writer.write(root);

        return true;
    }

    static bool DeserializeObject(JsonSerializable* pObj, std::string& input)
    {
        if(!pObj)
            return false;

        Json::Value root;
        Json::Reader reader;

        if(!reader.parse(input, root))
            return false;

        pObj->Deserialize(root);

        return true;
    }

    static void SerializeVectorIntoObjectValue(Json::Value &val, std::vector<std::string> &vec)        
    {                                                                                           
        if(val.isObject())                                                                      
        {                                                                                       
            std::vector<std::string>::iterator itr = vec.begin();                               
            for(; itr != vec.end(); itr++)                                                      
                val[*itr];                                                                      
        }                                                                                       
    }                                                                                           
                                                                                                
    static void SerializeVector(Json::Value &val, std::vector<std::string> &vec)                       
    {                                                                                           
        std::vector<std::string>::iterator itr = vec.begin();                                   
        for(; itr != vec.end(); itr++)                                                          
            val.append(*itr);                                                                   
    }                                                                                           

    static void DeserializeIntoVector(Json::Value &val, std::vector<std::string> &vec)
    {                                                                                                           
        vec.clear();                                                                                            
                                            
        Json::ValueIterator itr = val.begin();                                                                  
        for(; itr != val.end(); itr++)                                                                          
        {                                                                                                       
            vec.push_back((*itr).asString());                                                                   
        }                                                                                                       
    }                                                                                                           
                                                                                                            
    static void DeserializeObjectValueIntoVector(Json::Value &val, std::vector<std::string> &vec)
    {                                                                                                           
        if(val.isObject())                                                                                      
        {                                                                                                       
            vec.clear();                                                                                        
            Json::ValueIterator itr = val.begin();                                                              
                                                                                    
            for(; itr != val.end(); itr++)                                                                      
            {                                                                                                   
                vec.push_back(itr.key().asString());                                                            
            }                                                                                                   
        }                                                                                                       
    }

    static void SerializeMapIntoObject(Json::Value &val, std::map<std::string, std::string> &m)
    {
        if(val.isObject())
        {
            std::map<std::string, std::string>::iterator itr = m.begin();

            for(;itr != m.end(); itr++)
            {
                val[(*itr).first] = (*itr).second;
            }
        }
    }

    static void SerializeMapIntoObject(Json::Value &val, std::map<std::string, bool> &m)
    {
        if(val.isObject())
        {
            std::map<std::string, bool>::iterator itr = m.begin();

            for(;itr != m.end(); itr++)
            {
                val[(*itr).first] = (*itr).second;
            }
        }
    }

    static void DeserializeObjectValueIntoMap(Json::Value &val, std::map<std::string, std::string> &m)
    {
        if(val.isObject())
        {
            m.clear();
            Json::ValueIterator itr = val.begin();

            for(; itr != val.end(); itr++)                                                                      
            {
                m[itr.key().asString()] = (*itr).asString();
            }

        }
    }

    static void DeserializeObjectValueIntoMap(Json::Value &val, std::map<std::string, bool> &m)
    {
        if(val.isObject())
        {
            m.clear();
            Json::ValueIterator itr = val.begin();

            for(; itr != val.end(); itr++)                                                                      
            {
                m[itr.key().asString()] = (*itr).asBool();
            }

        }
    }

};

                                                                                                           

#endif

