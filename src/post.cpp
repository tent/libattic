#include "post.h"

#include <stdio.h>

void Version::Serialize(Json::Value& root) {
    root["id"] = id;
    root["type"] = type;
}

void Version::Deserialize(Json::Value& root) {
    id = root.get("id", "").asString();
    type = root.get("type", "").asString();
    published_at = root.get("published_at", "").asString();
    received_at = root.get("received_at", "").asString();
}

void Attachment::AssignKeyValue(const std::string &key, const Json::Value &val) {
    if(key == std::string("type")) {
        type = val.asString();
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
    
    std::cout<< "Uknown key : " << key << std::endl;
}

void Attachment::Serialize(Json::Value& root) {
    // TODO :: this, later
    std::cout<<" attatchment serialize UNIMPLEMENTED " << std::endl;
}

void Attachment::Deserialize(Json::Value& root) {
    type = root.get("type", "").asString();
    category = root.get("category", "").asString();
    name = root.get("name", "").asString();
    size = root.get("size", 0).asUInt();
}


Post::Post() {
    published_at_ = 0;
    received_at_ = 0;
    set_public(false);
}

Post::~Post() {
    if(attachments_.size() > 0) {
        std::cout<<" # attachments : " << attachments_.size() << std::endl;

        AttachmentVec::iterator itr = attachments_.begin();

        Attachment* pAtch=0;
        for(;itr != attachments_.end();) {
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

        attachments_.clear();
        std::cout<<" # attachments : " << attachments_.size() << std::endl;
    }
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

    if(attachments_.size() > 0) {
        // TODO::this
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

            attachments_.push_back(pAtch);
        }

    }


    if(!root["app"].isNull()) {
        tent_app_.Deserialize(root["app"]);
    }

    jsn::DeserializeObjectValueIntoMap(root["views"], views_);
    jsn::DeserializeObject(&permissions_,root["permissions"]);
}


