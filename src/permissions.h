
#ifndef PERMISSIONS_H_
#define PERMISSIONS_H_
#pragma once

#include "jsonserializable.h"

class Permissions : public JsonSerializable
{
public:
    Permissions()
    {
        m_Public = false;
    }

    ~Permissions() { } 

    void Serialize(Json::Value& root)
    {
        root["public"] = m_Public;
    }

    void Deserialize(Json::Value& root)
    {
        m_Public = root.get("public", false).asBool();
    }

    bool GetIsPublic() const { return m_Public; }
    void SetIsPublic(const bool pub) { m_Public = pub; }

private:
    bool    m_Public;

};




#endif

