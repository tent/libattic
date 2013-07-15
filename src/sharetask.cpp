#include "sharetask.h"

#include <iostream>

namespace attic { 

ShareTask::ShareTask(FileManager* fm, 
                     CredentialsManager* cm,
                     const AccessToken& at,
                     const Entity& entity,
                     const TaskContext& context)
                     :
                     TentTask(Task::SHARE,
                              fm,
                              cm,
                              at,
                              entity,
                              context) {
}

ShareTask::~ShareTask() {}

void ShareTask::RunTask() {
    std::cout<< " share task " << std::endl;
    // Retrieve public key
    // Decrypt Credentials from file post
    // encrypt credentials with public key
    // copy over chunks
    // create shared 
}

bool ShareTask::RerievePublicKey(const std::string& entity, std::string& out) {
    bool ret = false;

    return ret;
}

} //namespace 

