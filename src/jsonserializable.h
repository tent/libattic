
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
};

#endif

