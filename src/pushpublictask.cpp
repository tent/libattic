#include "pushpublictask.h"

#include <fstream>

#include "filesystem.h"
#include "crypto.h"
#include "netlib.h"

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
    std::string filepath = TentTask::filepath();
    int status = ret::A_FAIL_PATH_DOESNT_EXIST;
    if(fs::CheckFilepathExists(filepath)) {
        status = PushFile(filepath);
    }
    Callback(status, filepath);
    SetFinishedState();
}

int PushPublicTask::PushFile(const std::string& filepath) {
    int status = ret::A_OK;
    // Create Download Post
    DownloadPost dlp; 
    if(GenerateDownloadPost(filepath, dlp)) {
        // Upload file
        std::string url = entity()->GetPreferredServer().posts_feed();
        std::string protocol, host, path;
        netlib::ExtractHostAndPath(url, protocol, host, path);

        boost::asio::io_service ioserv;
        Connection* socket = new Connection(&ioserv);
        socket->Initialize(url);
        std::string boundary;
        utils::GenerateRandomString(boundary, 20);
        // Build header request
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        netlib::BuildRequestHeader("POST", url, boundary, &access_token(), request_stream); 
        // Start the request
        socket->Write(request);
        // Build Attachment Form
        std::string attachment_form;
        netlib::BuildAttachmentForm(dlp.filename(), dlp.file_size(), boundary, 0, attachment_form);
        request_stream << attachment_form;
        socket->Write(request);

        std::ifstream ifs;
        ifs.open(filepath.c_str(), std::ios::in | std::ios::binary);
        if(ifs.is_open()) {
            unsigned int step = 4000000;
            unsigned int total_read = 0;
            while(!ifs.eof()){
                unsigned int size = 0;
                if(total_read + step < dlp.filesize())
                    size = step;
                else
                    size = dlp.filesize() - total_read;

                char* buffer = new char[size];
                ifs.read(buffer, size);

            }
            ifs.close();
        }



        // Build Body Form 
        //
        // End Request
        //
        // Create public link
    }

    return status;
}


bool PushPublicTask::GenerateDownloadPost(const std::string& filepath, DownloadPost& out) {
    bool ret = false;

    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ios::in | std::ios::binary);
    if(ifs.is_open()) {
        // get size
        ifs.seekg (0, std::ifstream::end);
        unsigned int size = ifs.tellg();
        ifs.seekg (0, std::ifstream::beg);
        char size_buffer[256] = {'\0'};
        snprintf(size_buffer, 256, "%u", size);
        out.set_file_size(size_buffer);
        // extract filename
        size_t pos = filepath.rfind("/");
        if(pos != std::string::npos) {
            std::string filename = filepath.substr(pos);
            out.set_filename(filename);
        }

        // generate plaintext hash
        std::string hash;
        if(crypto::GeneratePlaintextHashForFile(filepath, hash)) {
            out.set_plaintext_hash(hash);
        }

        ifs.close();
    }

    return ret;
}



} // namespace

