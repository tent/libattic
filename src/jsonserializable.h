
#ifndef JSONSERIALIZABLE_H_
#define JSONSERIALIZABLE_H_
#pragma once

#include <json/json.h>

class JsonSerializable
{
public:
    virtual JsonSerializable() {};
    virtual ~JsonSerializable() {};

    virtual void Serialize(Json::Value& root) = 0;
    virtual void Deserialize(Json::Value& root) = 0;

};
#endif

