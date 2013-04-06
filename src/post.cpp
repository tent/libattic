#include "post.h"

#include <stdio.h>

namespace attic {

void Parent::Serialize(Json::Value& root) {
    root["version"] = version;
    root["entity"] = entity;
    root["original_entity"] = original_entity;
    root["post"] = post;
}

void Parent::Deserialize(Json::Value& root) {
    version = root.get("version", "").asString();
    entity = root.get("entity", "").asString();
    original_entity = root.get("original_entity", "").asString();
    post = root.get("post", "").asString();
}

void Version::Serialize(Json::Value& root) {
    root["id"] = id;
    root["type"] = type;

    if(parents.size()) {
        Json::Value parent_array(Json::arrayValue);
        for(unsigned int i=0; i<parents.size(); i++) {
            Json::Value val(Json::objectValue);
            jsn::SerializeObject(&parents[i], val);
            parent_array.append(val);
        }

        root["parents"] = parent_array;
    }
}

void Version::Deserialize(Json::Value& root) {
    id = root.get("id", "").asString();
    type = root.get("type", "").asString();
    published_at = root.get("published_at", "").asString();
    received_at = root.get("received_at", "").asString();

    Json::Value parent_array(Json::arrayValue);
    parent_array = root["parents"];

    if(parent_array.size()) {
        Json::ValueIterator itr = parent_array.begin();           
        for(;itr != parent_array.end(); itr++) {
            Json::Value val(Json::objectValue);
            val = *itr;

            Parent p;
            jsn::DeserializeObject(&p, val);
            parents.push_back(p);
        }
    }

}

void Attachment::AssignKeyValue(const std::string &key, const Json::Value &val) {
    if(key == std::string("content_type")) {
        content_type = val.asString();
        return;
    }
    if(key == std::string("category")) {
        category = val.asString();
        return;
    }
    if(key == std::string("name")) {
        name = val.asString();
        return;
    }
    if(key == std::string("size")) {
        size = val.asUInt();
        return;
    }
    if(key == "hash") {
        hash = val.asString();
        return;
    }
    if(key == "digest") {
        digest = val.asString();
        return;
    }
    
    std::cout<< "Uknown key : " << key << std::endl;
}

void Attachment::Serialize(Json::Value& root) {
    root["content_type"] = content_type;
    root["category"] = category;
    root["name"] = name;
    //root["hash"] = hash;
    root["digest"] = digest;
    //root["size"] = size; no need to send the size, the server calculates this
}

void Attachment::Deserialize(Json::Value& root) {
    content_type = root.get("content_type", "").asString();
    category = root.get("category", "").asString();
    name = root.get("name", "").asString();
    size = root.get("size", 0).asUInt();
    //hash = root.get("hash", "").asString();
    digest = root.get("digest", "").asString();
}

Post::Post() {
    published_at_ = 0;
    received_at_ = 0;
    set_public(false);
}

Post::~Post() {
    attachments_.clear();
}

void Post::get_content(const std::string& key, Json::Value& out) {
    ContentMap::iterator itr = content_.find(key);
    if(itr != content_.end())
        out = itr->second;
}

void Post::Serialize(Json::Value& root) {
    // General Post
    if(!id_.empty())
        root["id"] = id_;
    if(!entity_.empty())
        root["entity"] = entity_;

    if(published_at_ > 0)
        root["published_at"] = published_at_;

    if(mentions_.size() > 0) {
        Json::Value mentions;
        jsn::SerializeVector(mentions_, mentions);
        root["mentions"] = mentions;
    }
    
    if(licenses_.size() > 0) {
        Json::Value licenses;
        jsn::SerializeVector(licenses_, licenses);
        root["licenses"] = licenses;
    }

    if(!type_.empty())
        root["type"] = type_;
   
    if(content_.size() > 0) {
        Json::Value content(Json::objectValue);
        jsn::SerializeMapIntoObject(content, content_);
        root["content"] = content;
    }

    std::cout<<" ATTACHMENT SERIALIZE SIZE : " << attachments_.size() << std::endl;
    if(attachments_.size() > 0) {
        Json::Value attachment_arr(Json::arrayValue);
        AttachmentMap::iterator itr = attachments_.begin();
        for(;itr!= attachments_.end(); itr++) {
            Json::Value attachment(Json::objectValue);
            itr->second.Serialize(attachment);
            attachment_arr.append(attachment);
        }

        root["attachments"] = attachment_arr;
    }

    /*
    Json::Value app;
    tent_app_.Serialize(app); 
    root["app"] = app;
    */

    if(views_.size() > 0) {
        Json::Value views(Json::objectValue);
        jsn::SerializeMapIntoObject(views, views_);
        root["views"] = views;
    }

    Json::Value permissions(Json::objectValue);
    jsn::SerializeObject(&permissions_, permissions);
    root["permissions"] = permissions;
}

void Post::Deserialize(Json::Value& root) {
    // General Post
    id_             = root.get("id", "").asString();
    entity_         = root.get("entity", "").asString();
    std::string pub = root.get("published_at", "").asString();
    published_at_   = atoi(pub.c_str());
    std::string rec = root.get("received_at", "").asString();

    received_at_    = atoi(rec.c_str());

    jsn::DeserializeObject(&version_, root["version"]);
    jsn::DeserializeIntoVector(root["mentions"], mentions_);
    jsn::DeserializeIntoVector(root["licenses"], licenses_);

    jsn::DeserializeObjectValueIntoMap(root["content"], content_);
    
    // Deserialize this into an array of objects
    Json::Value jsn_attch(Json::arrayValue);
    jsn_attch = root["attachments"];

    if(jsn_attch.size() > 0) {
        Json::ValueIterator itr = jsn_attch.begin();           
        for(; itr != jsn_attch.end(); itr++) {
            //Attachment* pAtch = new Attachment;
            Attachment attch;
            Json::Value aobj(Json::objectValue);
            aobj = (*itr);

            if(aobj.isObject()) {
                if(aobj.size() >= 4) {
                    Json::ValueIterator ii = aobj.begin();
                    for(; ii != aobj.end(); ii++) {
                        attch.AssignKeyValue(ii.key().asString(), (*ii));
                    }
                }
            }
            PushBackAttachment(attch);
        }
    }


    if(!root["app"].isNull()) {
        tent_app_.Deserialize(root["app"]);
    }

    jsn::DeserializeObjectValueIntoMap(root["views"], views_);
    jsn::DeserializeObject(&permissions_,root["permissions"]);
}

bool Post::has_attachment(const std::string& name) {
    if(attachments_.find(name) != attachments_.end())
        return true;
    return false;
}

const Attachment& Post::get_attachment(const std::string& name) {
    return attachments_[name];
}



}//namespace
