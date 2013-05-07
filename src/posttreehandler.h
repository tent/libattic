#ifndef POSTTREEHANDLER_H_
#define POSTTREEHANDLER_H_
#pragma once

#include <string>
#include "posttree.h"

namespace attic { 

class PostTreeHandler {
public:
    PostTreeHandler(const std::string& post_path);
    ~PostTreeHandler();

    int ConstructPostTree(const std::string& post_id, PostTree& out);

private:
    std::string post_path_;

};

} // namespace
#endif

