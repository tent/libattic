#include "posttreehandler.h"

#include "errorcodes.h"

namespace attic { 

PostTreeHandler::PostTreeHandler(const std::string& post_path) {
    post_path_ = post_path;
}

PostTreeHandler::~PostTreeHandler() {

}

int ConstructPostTree(const std::string& post_id, PostTree& out) {
    int status = ret::A_OK;

    return status;
}

} //namespace
