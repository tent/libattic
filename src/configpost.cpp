#include "configpost.h"

#include "constants.h"

namespace attic {

ConfigPost::ConfigPost() {
    set_type(cnst::g_attic_config_type);
}

ConfigPost::~ConfigPost() {

}

void ConfigPost::Serialize(Json::Value& root) {
    Json::Value config(Json::arrayValue);
    std::map<std::string, ConfigValue>::iterator itr = config_map_.begin();
    for(;itr!= config_map_.end(); itr++) {
        Json::Value config_val(Json::objectValue);
        config_val["type"] = itr->second.type;
        config_val["key"] = itr->second.key;
        config_val["value"] = itr->second.value;
        config.append(config_val);
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
        Json::Value config_value(Json::objectValue);
        config_value = *itr;
        ConfigValue v;
        v.type = config_value.get("type", "").asString();
        v.key = config_value.get("key", "").asString();
        v.value = config_value.get("value", "").asString();
        config_map_[v.key] = v;
    }
}

void ConfigPost::PushBackConfigValue(const std::string& type,
                                     const std::string& key, 
                                     const std::string& value) {
    ConfigValue v;
    v.type = type;
    v.key = key;
    v.value = value;
    config_map_[key] = v;
}

}//namespace

