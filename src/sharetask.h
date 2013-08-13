#ifndef SHARETASK_H_
#define SHARETASK_H_
#pragma once

#include <string>
#include "tenttask.h"
#include "credentials.h"

namespace attic {

class ShareTask : public TentTask {
    bool RetrievePublicKey(const std::string& entity_url, std::string& out);
    bool RetrieveDecryptedCredentials(const std::string& post_id, Credentials& out);
    bool CreateSharedFilePost(const std::string& file_post_id,
                              const std::string& entity_url,
                              const std::string& public_key);

public:
    ShareTask(FileManager* fm, 
              CredentialsManager* cm,
              const AccessToken& at,
              const Entity& entity,
              const TaskContext& context);
    ~ShareTask();

    virtual void OnStart() {}
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};

} // namespace
#endif

