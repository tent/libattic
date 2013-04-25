#include "censuspost.h"

#include "constants.h"

namespace attic { 

CensusPost::CensusPost() {
    set_type(cnst::g_attic_census_type);
}

CensusPost::~CensusPost() {}

void CensusPost::Serialize(Json::Value& root){

    Post::Serialize(root);
}

void CensusPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);
}
    

} //namespace
