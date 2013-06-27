#include "pushpublictask.h"

#include <fstream>

#include "filesystem.h"
#include "crypto.h"
#include "netlib.h"
#include "connectionhandler.h"
#include "envelope.h"
#include "utils.h"


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
        DownloadPost dlp;
        status = PushFile(filepath, dlp);
        if(status == ret::A_OK) {
            std::string link_out;
            GeneratePublicLink(dlp, link_out);
            CallbackWithUrl(ret::A_OK, link_out, "");
        }
    }
    //Callback(status, filepath);
    SetFinishedState();
    std::cout<<" finished push public task with status : " << status << std::endl;
}

int PushPublicTask::PushFile(const std::string& filepath, DownloadPost& out) {

    // TODO :: Optimize this with a file streamer,
    // abstract more of the parts and chunked encoding
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
        netlib::BuildRequestHeader("POST", url, boundary, &access_token(), oss); 
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

        std::string body_form, chunked_body_form;
        netlib::BuildBodyForm(dlp.type(), body, boundary, body_form);
        netlib::ChunkString(body_form, chunked_body_form);

        std::cout<<" request body :\n\n " << body_form << std::endl;
        std::cout<<" chunked request body : \n\n"<< chunked_body_form <<std::endl;
        std::cout<<" \n\nchunked body size : " << chunked_body_form.size();

        boost::asio::streambuf c_requestBody;
        std::ostream c_part_stream(&c_requestBody);
        c_part_stream << chunked_body_form;

        try {
            socket->Write(c_requestBody);
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

        std::string a_form, a_form_chunked;
        netlib::BuildAttachmentForm(dlp.filename(), dlp.file_size(), boundary, 0, a_form);
        
        netlib::ChunkString(a_form, a_form_chunked);
        std::cout<<" attachemnt form :\n\n " << a_form << std::endl;

        boost::asio::streambuf c_att_request;
        std::ostream c_att_request_stream(&c_att_request);
        c_att_request_stream << a_form_chunked;
        socket->Write(c_att_request);

        WriteOnceFileToConnection(filepath, socket);

        std::cout<<" ending request " << std::endl;
        std::ostringstream end;
        end <<"\r\n--"<< boundary << "--\r\n\r\n";
        std::string chunk_end;
        netlib::ChunkEndString(end.str(), chunk_end);

        std::cout <<" chunked end : \n\n" << chunk_end;

        boost::asio::streambuf end_request;
        std::ostream end_request_stream(&end_request);
        end_request_stream << chunk_end;

        socket->Write(end_request);

        std::cout<<" reading response " << std::endl;
        Response resp;
        socket->InterpretResponse(resp);

        std::cout<<" RESPONSE CODE : " << resp.code << std::endl;
        std::cout<<" HEADERS : " << resp.header.asString() << std::endl;
        std::cout<<" BODY : " << resp.body << std::endl;

        Envelope env;
        jsn::DeserializeObject(&env, resp.body);
        post::DeserializePostIntoObject(env.post(), &out);

        delete socket;
        socket = NULL;
    }

    return status;
}

void PushPublicTask::WriteOnceFileToConnection(const std::string& filepath, Connection * con) {
        std::ifstream ifs;
        ifs.open(filepath.c_str(), std::ios::in | std::ios::binary);
        if(ifs.is_open()) {
            ifs.seekg (0, std::ifstream::end);
            unsigned int filesize = ifs.tellg();
            ifs.seekg (0, std::ifstream::beg);

            char* buffer = new char[filesize];
            std::string hold;
            hold.append(buffer, filesize);

            boost::asio::streambuf binbuf;
            std::ostream part(&binbuf);

            std::string chunked_hold;
            netlib::ChunkString(hold, chunked_hold);
            part << chunked_hold;

            con->Write(binbuf);
        }
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
            while(total_read < filesize){
                unsigned int size = 0;
                if(total_read + step < filesize)
                    size = step;
                else
                    size = filesize - total_read;

                char* buffer = new char[size];
                ifs.read(buffer, size);
                total_read += size;

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

