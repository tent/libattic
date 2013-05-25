#ifndef TREEHANDLER_H_
#define TREEHANDLER_H_
#pragma once

#include <string>
#include "posttree.h"

class TreeHandler() {
    bool RetrievePost(const std::string& post_url, std::string& next_id, FilePost& out);
public:
    TreeHandler(const std::string& post_path);
    ~TreeHandler();

    bool ConstructPostTree(const std::string& post_id, PostTree& out);
private:
    std::string post_path_
};
#endif

