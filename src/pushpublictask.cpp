#include "pushpublictask.h"

#include <fstream>

#include "filesystem.h"
#include "crypto.h"
#include "netlib.h"
#include "connectionhandler.h"


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
    int status = ret::A_FAIL_PATH_DOESNT_EXIST;
    if(fs::CheckFilepathExists(filepath)) {
        status = PushFile(filepath);
    }
    Callback(status, filepath);
    SetFinishedState();
    std::cout<<" finished push public task with status : " << status << std::endl;
}

int PushPublicTask::PushFile(const std::string& filepath) {
    int status = ret::A_OK;
    // Create Download Post
    DownloadPost dlp; 
    if(GenerateDownloadPost(filepath, dlp)) {
        // Upload file
        std::string url = entity()->GetPreferredServer().posts_feed();
        std::cout<<" CONNECTING TO URL : " << url << std::endl;
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

        std::ostringstream oss;
        netlib::BuildRequestHeaderNotChunked("POST", url, boundary, &access_token(), oss); 
        std::cout<<" Request header :\n\n " << oss.str() << std::endl;
        request_stream << oss.str();
        // Start the request
        std::cout<<" starting the request " << std::endl;
        try {
            socket->Write(request);
        }
        catch (std::exception& e) {
            std::cout<<" EXCEPTION : " << e.what() << std::endl;
            Response fail;
            socket->InterpretResponse(fail);
            std::cout<<"FAIL REPSONSE : " << fail.code << std::endl;
            std::cout<<"FAIL HEADER : " << fail.header.asString() << std::endl;
            std::cout<<"FAIL BODY : " << fail.body << std::endl;
        }
        // Build Body Form 
        std::string body;
        jsn::SerializeObject(&dlp, body);

        boost::asio::streambuf requestBody;
        std::ostream part_stream(&requestBody);
        netlib::BuildBodyForm(dlp.type(), body, boundary, part_stream);

        std::ostringstream pbuf;
        pbuf << &requestBody;
        std::cout<< " request body :\n\n " << pbuf.str();
        part_stream << pbuf.str();

        try {
            socket->Write(requestBody);
        }
        catch (std::exception& e) {
            std::cout<<" EXCEPTION 0: " << e.what() << std::endl;
            Response fail;
            socket->InterpretResponse(fail);
            std::cout<<"FAIL REPSONSE : " << fail.code << std::endl;
            std::cout<<"FAIL HEADER : " << fail.header.asString() << std::endl;
            std::cout<<"FAIL BODY : " << fail.body << std::endl;
        }
        // Build Attachment Form
        std::cout<<" building attachment form " << std::endl;
        boost::asio::streambuf att_request;
        std::ostream att_request_stream(&att_request);
        std::string attachment_form;
        netlib::BuildAttachmentForm(dlp.filename(), dlp.file_size(), boundary, 0, attachment_form);
        att_request_stream << attachment_form;

        std::cout<<" attachemnt form :\n\n " << attachment_form << std::endl;
        socket->Write(att_request);

        std::cout<<" writing file to connection " << std::endl;
        try {
            WriteFileToConnection(filepath, socket);
        }
        catch(std::exception& e) {
            std::cout<<" EXECEPTION 1: " << e.what() << std::endl;
            Response fail;
            socket->InterpretResponse(fail);
            std::cout<<"FAIL REPSONSE : " << fail.code << std::endl;
            std::cout<<"FAIL HEADER : " << fail.header.asString() << std::endl;
            std::cout<<"FAIL BODY : " << fail.body << std::endl;
        }


        //netlib::AddEndBoundry(part_stream, boundary);
        std::cout<<" ending request " << std::endl;
        std::ostringstream end;
        end <<"\r\n--"<< boundary << "--\r\n\r\n";
        boost::asio::streambuf end_request;
        std::ostream end_request_stream(&end_request);
        end_request_stream << end.str();

        socket->Write(end_request);

        std::cout<<" reading response " << std::endl;
        Response resp;
        socket->InterpretResponse(resp);

        std::cout<<" RESPONSE CODE : " << resp.code << std::endl;
        std::cout<<" HEADERS : " << resp.header.asString() << std::endl;
        std::cout<<" BODY : " << resp.body << std::endl;


        delete socket;
        socket = NULL;
        //
        // Create public link
    }

    return status;
}

void PushPublicTask::WriteFileToConnection(const std::string& filepath, Connection* con) {
        std::ifstream ifs;
        ifs.open(filepath.c_str(), std::ios::in | std::ios::binary);
        if(ifs.is_open()) {
            ifs.seekg (0, std::ifstream::end);
            unsigned int filesize = ifs.tellg();
            ifs.seekg (0, std::ifstream::beg);

            unsigned int step = 4000000;
            unsigned int total_read = 0;
            while(!ifs.eof()){
                unsigned int size = 0;
                if(total_read + step < filesize)
                    size = step;
                else
                    size = filesize - total_read;

                char* buffer = new char[size];
                ifs.read(buffer, size);

                std::string buf;
                buf.append(buffer, size);

                delete[] buffer;
                buffer = NULL;

                boost::asio::streambuf binbuf;
                std::ostream part(&binbuf);
                part << buf;
                std::cout<<" writing : " << buf.size() <<" bytes "<< std::endl;
                con->Write(binbuf);
            }
            ifs.close();
        }
}

bool PushPublicTask::GenerateDownloadPost(const std::string& filepath, DownloadPost& out) {
    bool ret = false;

    std::cout<<" generating download post for : " << filepath << std::endl;
    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ios::in | std::ios::binary);
    if(ifs.is_open()) {
        // get size
        ifs.seekg (0, std::ifstream::end);
        unsigned int size = ifs.tellg();
        ifs.seekg (0, std::ifstream::beg);
        std::cout << " file size : "<< size << std::endl;
        char size_buffer[256] = {'\0'};
        snprintf(size_buffer, 256, "%u", size);
        out.set_file_size(size_buffer);
        // extract filename
        size_t pos = filepath.rfind("/");
        if(pos != std::string::npos) {
            std::string filename = filepath.substr(pos+1);
            out.set_filename(filename);
        }

        // generate plaintext hash
        std::string hash;
        if(crypto::GeneratePlaintextHashForFile(filepath, hash)) {
            out.set_plaintext_hash(hash);
        }

        ifs.close();
        ret = true;
    }

    return ret;
}



} // namespace

