#include "pushpublictask.h"

#include <fstream>

#include "filesystem.h"
#include "crypto.h"
#include "netlib.h"
#include "connectionhandler.h"
#include "envelope.h"
#include "utils.h"

#include "plainfileupload.h"


namespace attic { 

PushPublicTask::PushPublicTask(FileManager* fm,
                               CredentialsManager* cm,
                               const AccessToken& at,
                               const Entity& entity,
                               const TaskContext& context)
                               :
                               TentTask(Task::PUSHPUBLIC,
                                        fm,
                                        cm,
                                        at,
                                        entity,
                                        context) 
{
}

PushPublicTask::~PushPublicTask() {}

void PushPublicTask::RunTask() {
    std::cout<<" starting push public task " << std::endl;
    std::string filepath = TentTask::filepath();
    std::cout<<" creating public download link for : " << filepath << std::endl;
    int status = ret::A_FAIL_PATH_DOESNT_EXIST;
    if(fs::CheckFilepathExists(filepath)) {
        // Add a filesize check, limit the allowable filesize
        std::string link_out, error_str;
        DownloadPost dlp;

        std::string url = entity()->GetPreferredServer().posts_feed();
        PlainFileUpload pfu(access_token());
        if(pfu.Upload(url, filepath, dlp)) {
            GeneratePublicLink(dlp, link_out);
            status = ret::A_OK;
        }

        CallbackWithUrl(status, link_out, error_str);
    }
    //Callback(status, filepath);
    SetFinishedState();
    std::cout<<" finished push public task with status : " << status << std::endl;
}

void PushPublicTask::GeneratePublicLink(DownloadPost& in, std::string& link_out) {

    std::cout<<" id : " << access_token().hawk_key() << std::endl;
    time_t t = time(0);
    t+=10000000; // set expiration from a year from now
    char tbuf[256]={'\0'};
    snprintf(tbuf, (sizeof(time_t)*256), "%ld", t);

    std::string url = entity()->GetPreferredServer().attachment();
    std::string posturl;
    utils::FindAndReplace(url, "{entity}", entity()->entity(), posturl);
    utils::FindAndReplace(posturl, "{digest}", in.attachments()->begin()->second.digest, posturl);

    std::string name = in.attachments()->begin()->second.name;
    utils::FindAndReplace(posturl, "{name}", name, posturl);

    std::cout<<" POST URL : " << posturl << std::endl;
    std::string normalized_string;
    netlib::SignRequest(posturl, 
                        "GET", 
                        "hawk.1.bewit", 
                        "", 
                        t, 
                        access_token().access_token(),
                        access_token().hawk_key(),
                        "", //access_token().app_id(),
                        normalized_string);

    std::cout<<" NORMALIZED STRING : " << normalized_string << std::endl;

    std::string ts = tbuf;
    std::cout<<" time stamp : " << ts << std::endl;
    std::ostringstream bewit;
    bewit << access_token().access_token() << "\\" << ts << "\\" << normalized_string << "\\";
    std::cout<<" bewit : " << bewit.str() << std::endl;

    std::string hbewit;
    crypto::Base64EncodeString(bewit.str(), hbewit);

    utils::FindAndReplace(hbewit, "+", "-", hbewit);
    utils::FindAndReplace(hbewit, "/", "_", hbewit);

    size_t pos = 0;
    for(;;) {
        pos = hbewit.find("=");
        if(pos != std::string::npos) {
            hbewit.erase(pos, 1);
        }
        else 
            break;
    }

    std::cout<<" hashed bewit : " << hbewit << std::endl;

    std::cout<<" complete url : " << posturl << "?bewit=" << hbewit << std::endl;

    link_out = posturl + "?bewit=" + hbewit;
}


} // namespace

