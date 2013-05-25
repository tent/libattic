#ifndef POST_H_
#define POST_H_
#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "jsonserializable.h"
#include "permissions.h"
#include "tentapp.h"

namespace attic {

struct Mention : public JsonSerializable {
    std::string entity;     // Required
    std::string original_entity;
    std::string post;
    std::string version;
    std::string type;
    bool is_public;

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);
};

struct Parent : public JsonSerializable {
    std::string version; // Post version identifier - required
    std::string entity; // Entity of parent post - optional
    std::string original_entity; // original parent post - optional
    std::string post; // identifier of parent post - optional (should not be optional)

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);
};

class Version : public JsonSerializable {
public:
    typedef std::vector<Parent> ParentList;

    const std::string& id() const           { return id_; }
    const std::string& published_at() const { return published_at_; }
    const std::string& received_at() const  { return received_at_; } 

    void set_id(const std::string& id)              { id_ = id; }
    void set_published_at(const std::string& at)    { published_at_ = at; }
    void set_received_at(const std::string& at)     { received_at_ = at; }

    void PushBackParent(Parent& p) { parents_.push_back(p); }

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    const ParentList& parents() const { return parents_; }
private:
    ParentList parents_;
    std::string id_; // Post version identifier
    std::string published_at_;
    std::string received_at_;
};

struct Attachment : public JsonSerializable {
    std::string content_type;      // `json:"content_type"`
    std::string category;          // `json:"category"`
    std::string name;              // `json:"name"`
    std::string hash;              // `json:"hash"`
    std::string digest;            // `json:"digest"`
    unsigned int size;             // `json:"size"`

    void AssignKeyValue(const std::string &key, const Json::Value &val);
    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);
};

class Post : public JsonSerializable {
public:
    typedef std::map<std::string, Attachment> AttachmentMap;
    typedef std::vector<Mention> MentionsList;

    Post();
    ~Post();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void get_content(const std::string& key, Json::Value& out);

    unsigned int attachments_count()            { return attachments_.size(); }
    AttachmentMap* attachments()                { return &attachments_; }
    MentionsList* mentions()                    { return &mentions_; }

    bool has_attachment(const std::string& name);
    const Attachment& get_attachment(const std::string& name);

    const std::string& id() const       { return id_; }
    const std::string& entity() const   { return entity_; }
    const std::string& type() const     { return type_; }
    unsigned int published_at() const   { return published_at_; }
    unsigned int received_at() const    { return received_at_; }
    const Version& version() const      { return version_; }

    const std::string& version_id() const { return version_.id(); }

    void set_id(const std::string &id)              { id_ = id; }
    void set_entity(const std::string &entity)      { entity_ = entity; }
    void set_published_at(unsigned int uUnixTime)   { published_at_ = uUnixTime; }
    void set_type(const std::string &type)          { type_ = type + "#";  base_type_ = type + "#"; }
    void set_content(const std::string &type, Json::Value &val) { content_[type] = val; }
    void set_public(const bool pub)                 { permissions_.SetIsPublic(pub); }

    void PushBackAttachment(Attachment& att) { attachments_[att.name] = att; }
    void PushBackMention(const Mention& mention) { mentions_.push_back(mention); }

    void PushBackParent(Parent& p) { version_.PushBackParent(p); }

    void MentionPost(const std::string& entity, const std::string& postid);

    void set_fragment(const std::string& fragment);
    void clear_fragment();
private:
    typedef std::map<std::string, Json::Value> ContentMap;

    std::string                         id_;
    std::string                         entity_;
    std::string                         base_type_; // base type of post
    std::string                         type_; // type (may be fragmented)
    unsigned int                        published_at_;
    unsigned int                        received_at_;
    std::vector<std::string>            licenses_;
    ContentMap                          content_;
    AttachmentMap                       attachments_;
    MentionsList                        mentions_;
    TentApp                             tent_app_;
    std::map<std::string, std::string>  views_;
    Permissions                         permissions_;
    Version                             version_;
};

}//namespace
#endif

