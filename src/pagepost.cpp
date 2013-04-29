#include "pagepost.h"


namespace attic { 

void Pages::Serialize(Json::Value& root) {
    root["first"] = first_;
    root["prev"] = prev_;
    root["next"] = next_;
    root["last"] = last_;
}

void Pages::Deserialize(Json::Value& root) {
    first_ = root.get("first", "").asString();
    prev_ = root.get("prev", "").asString();
    next_ = root.get("next", "").asString();
    last_ = root.get("last", "").asString();
}

void PagePost::Serialize(Json::Value& root) {

}

void PagePost::Deserialize(Json::Value& root) {
    pages_.Deserialize(root["pages"]);
    jsn::SerializeJsonValue(root["data"], data_);
}


}//namespace
