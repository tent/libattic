#include "configpost.h"

#include "constants.h"

namespace attic {

ConfigPost::ConfigPost() {
    set_type(cnst::g_attic_config_type);
}

ConfigPost::~ConfigPost() {

}

void ConfigPost::Serialize(Json::Value& root) {
    Json::Value config(Json::objectValue);
    std::map<std::string, std::string>::iterator itr = config_map_.begin();
    for(;itr!= config_map_.end(); itr++) {
        config[itr->first] = itr->second;
    }
    set_content("config", config);

    Post::Serialize(root);
}

void ConfigPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    Json::Value config(Json::objectValue);
    get_content("config", config);

    config_map_.clear();
    Json::ValueIterator itr = config.begin();
    for(;itr!= config.end(); itr++) {
        std::cout<<" key : " << itr.key().asString() << std::endl;
        std::cout<<" val : " << (*itr).asString() << std::endl;
        config_map_[itr.key().asString()] = (*itr).asString();
    }
}

void ConfigPost::PushBackConfigValue(const std::string& key, const std::string& value) {
    config_map_[key] = value;
}

}//namespace

