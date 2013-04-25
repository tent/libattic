#ifndef CENSUSPOST_H_
#define CENSUSPOST_H_
#pragma once

#include "post.h"

namespace attic { 

class CensusPost : public Post { 
public:
    CensusPost();
    ~CensusPost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);
private:
};

}//namespace
#endif

