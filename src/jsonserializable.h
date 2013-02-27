
#ifndef JSONSERIALIZABLE_H_
#define JSONSERIALIZABLE_H_
#pragma once

#include <string>
#include <json/json.h>

#include <iostream>

#include <stdio.h>
#include <string.h>

class JsonSerializable
{
public:
    JsonSerializable() {};
    virtual ~JsonSerializable(){};

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

        Json::Value root(Json::nullValue);
        pObj->Serialize(root);

        Json::StyledWriter writer;
        output = writer.write(root);

        return true;
    }


    static bool SerializeObject(JsonSerializable* pObj, Json::Value &val)
    {
        if(pObj && val.isObject())
        {
            pObj->Serialize(val);
            return true;
        }
        return false;
    }

    static bool SerializeJsonValue(Json::Value& root, std::string& output)
    {
        Json::StyledWriter writer;
        output = writer.write(root);
        return true;
    }

    static bool DeserializeJsonValue(Json::Value& val, std::string& input)
    {
        Json::Reader reader;
        if(!reader.parse(input, val))
            return false;
        return true;
    }

    static bool DeserializeObject(JsonSerializable* pObj, const std::string& input)
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

    static bool DeserializeObject(JsonSerializable* pObj, Json::Value& val)
    {
        if(pObj && val.isObject())
        {
            pObj->Deserialize(val);
            return true;
        }

        return false;
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
        for(; itr != val.end(); itr++) { 
            if((*itr).isConvertibleTo(Json::stringValue))
                vec.push_back((*itr).asString());
        }
    }

    static void DeserializeObjectValueIntoVector(Json::Value &val, std::vector<std::string> &vec)
    {
        if(val.isObject()) {
            vec.clear();
            Json::ValueIterator itr = val.begin();
                                                                                    
            for(; itr != val.end(); itr++) { 
                vec.push_back(itr.key().asString());
            }
        }
    }

    static void SerializeMapIntoObject(Json::Value &val, std::map<std::string, std::string> &m)
    {
        if(val.isObject()) {
            std::map<std::string, std::string>::iterator itr = m.begin();

            for(;itr != m.end(); itr++) {
                val[(*itr).first] = (*itr).second;
            }

        }
    }

    static void SerializeMapIntoObject(Json::Value &val, std::map<std::string, bool> &m)
    {
        if(val.isObject()) {
            std::map<std::string, bool>::iterator itr = m.begin();

            for(;itr != m.end(); itr++) {
                val[(*itr).first] = (*itr).second;
            }
        }
    }

    static void DeserializeObjectValueIntoMap(Json::Value &val, std::map<std::string, std::string> &m)
    {
        if(val.isObject()){
            m.clear();
            Json::ValueIterator itr = val.begin();

            for(; itr != val.end(); itr++) {
                printf( " key type=[%d]", itr.key().type());
                printf( " value type=[%d]\n", (*itr).type());

                if((*itr).type() == 6) {
                    Json::Value arr(Json::arrayValue);
                    arr = (*itr);

                    std::cout<<" size : " << arr.size() << std::endl;
                                                                            
                    std::cout << "parsing array " << std::endl;
                    Json::ValueIterator itr2 = arr.begin();

                    for(; itr2 != arr.end(); itr2++) {
                        std::cout<< (*itr2).asString() << std::endl;
                    }
                }

                std::cout << itr.key().asString() << std::endl; 
                std::cout << (*itr).asString() << std::endl;

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

