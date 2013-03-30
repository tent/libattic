#ifndef ENTITYPOST_H_
#define ENTITYPOST_H_
#pragma once

#include "post.h"
#include "entity.h"

namespace attic { 

class EntityPost : public Post {
public:
    EntityPost();
    ~EntityPost();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    void set_entity(const Entity& ent) { entity_ = ent; }
    const Entity& entity() const { return entity_; }
private:
    Entity entity_;
};

}//namespace
#endif

