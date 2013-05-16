#include "atticservice.h"

#include "utils.h"
#include "configmanager.h"
#include "constants.h"
#include "filesystem.h"
#include "errorcodes.h"

namespace attic {

AtticService::AtticService() {
    service_manager_ = NULL;
    task_manager_ = NULL;
    thread_manager_ = NULL;
    client_ = NULL;
}

AtticService::~AtticService() {

}

int AtticService::start() {
    utils::SeedRand();
    LoadConfigValues();
    ValidateDirectories();

}

void AtticService::LoadConfigValues() {
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigWorkingDir, working_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigConfigDir, config_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigTempDir, temp_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigEntityURL, entity_url_);
}

void AtticService::ValidateDirectories() {
    if(!working_dir_.empty())
        fs::CreateDirectory(working_dir_);
    if(!config_dir_.empty())
        fs::CreateDirectory(config_dir_);
    if(!temp_dir_.empty())
        fs::CreateDirectory(temp_dir_);
}

int AtticService::stop() {

}

}// namespace
