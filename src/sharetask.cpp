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
    // Decrypt Credentials from file post
    // Retrieve public key
    // encrypt credentials
    // create shared 

}


} //namespace 

