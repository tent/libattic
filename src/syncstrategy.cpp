#include "syncstrategy.h"

namespace attic { 

SyncStrategy::SyncStrategy() {}
SyncStrategy::~SyncStrategy() {}

int SyncStrategy::Execute(FileManager* pFileManager,
                          CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);
    // TODO :: this, later


    return status;
}

}//namespace

