#include "plainfileupload.h"

#include "netlib.h"
#include "envelope.h"

namespace attic { 

PlainFileUpload::PlainFileUpload(const AccessToken& at) {
    access_token_ = at;
    con_ = NULL;
}

PlainFileUpload::~PlainFileUpload() {
    if(con_) {
        delete con_;
        con_ = NULL;
    }
}

bool PlainFileUpload::Upload(const std::string& url,
                             const std::string& filepath,
                             DownloadPost& out) {
    // TODO ::
    // detect whether a file or folder
    // if file 
    //      just upload as per usual
    // if folder
    //      calculate all folder contents, folders, sub folders etc
    //      if less than 1gb
    //          zip and upload
    bool ret = false;
    url_ = url;
    filepath_ = filepath;

    if(BeginRequest()) {
        DownloadPost dlp;
        if(UploadPostBody(dlp)) {
            if(UploadFile(filepath_,
                          dlp.filename(),
                          dlp.filesize())) {
                    if(EndRequest())
                        ret = InterpretResponse(out);
            }
        }
    }

    return ret; 
}
bool PlainFileUpload::BeginRequest() {
    bool ret = false;
    std::string protocol, host, path;
    netlib::ExtractHostAndPath(url_, protocol, host, path);

    con_ = new Connection(&io_service_);
    if(con_->Initialize(url_) == ret::A_OK)  {
        utils::GenerateRandomString(boundary_, 20);
        // Build header request
        boost::asio::streambuf request;
        std::ostream request_stream(&request);

        std::ostringstream oss;
        netlib::BuildRequestHeader("POST", url_, boundary_, &access_token_, oss); 
        std::cout<<" Request header :\n\n " << oss.str() << std::endl;
        request_stream << oss.str();
        ret = Push(request);
    }
    return ret;
}

bool PlainFileUpload::UploadPostBody(DownloadPost& out) {
    bool ret = false;
    // Build Body Form 
    if(GenerateDownloadPost(filepath_, out)) {
        std::string body;
        jsn::SerializeObject(&out, body);

        std::string body_form, chunked_body_form;
        netlib::BuildBodyForm(out.type(), body, boundary_, body_form);
        netlib::ChunkString(body_form, chunked_body_form);

        boost::asio::streambuf chunked_request;
        std::ostream c_part_stream(&chunked_request);
        c_part_stream << chunked_body_form;
        ret = Push(chunked_request);
    }
    return ret;
}

bool PlainFileUpload::UploadFile(const std::string& filepath,
                                 const std::string& filename,
                                 const std::string& filesize) {
    //Note* we are going to manually chunk encode this and then stream
    //      the file up to the server in one large chunk
    bool ret = false;
    // Build attachment form
    std::string attachment_form;
    netlib::BuildAttachmentForm(filename, filesize, boundary_, 0, attachment_form);

    std::cout<<" attachment form : " << attachment_form << std::endl;

    unsigned int total_size = attachment_form.size() + atoi(filesize.c_str());
    std::cout<<" TOAL SIZE : " << total_size << std::endl;
    std::ostringstream chunked_attachment_form;
    chunked_attachment_form << std::hex << total_size;
    chunked_attachment_form << "\r\n" << attachment_form;

    // push up
    boost::asio::streambuf request;
    std::ostream part_stream(&request);
    part_stream << chunked_attachment_form.str();
    if(Push(request)) {
        // begin streaming 
        if(WriteFileToConnection(filepath)) {
            // end the chunk
            boost::asio::streambuf end_request;
            std::ostream end_part_stream(&end_request);
            end_part_stream << "\r\n";
            ret = Push(end_request);
        }
    }
    return ret;
}

bool PlainFileUpload::EndRequest() {
    bool ret = false;
    std::ostringstream end;
    end <<"\r\n--"<< boundary_ << "--\r\n\r\n";
    std::string chunk_end;
    netlib::ChunkEndString(end.str(), chunk_end);

    std::cout <<" chunked end : \n\n" << chunk_end;

    boost::asio::streambuf end_request;
    std::ostream end_request_stream(&end_request);
    end_request_stream << chunk_end;

    ret = Push(end_request);
    return ret;
}

bool PlainFileUpload::InterpretResponse(DownloadPost& out) {
    bool ret = false;
    Response resp;
    con_->InterpretResponse(resp);

    if(resp.code == 200) {
        std::cout<<" RESPONSE CODE : " << resp.code << std::endl;
        std::cout<<" HEADERS : " << resp.header.asString() << std::endl;
        std::cout<<" BODY : " << resp.body << std::endl;

        Envelope env;
        jsn::DeserializeObject(&env, resp.body);
        post::DeserializePostIntoObject(env.post(), &out);
        ret = true;
    }
    return ret;
}


bool PlainFileUpload::Push(boost::asio::streambuf& request) {
    bool ret = false;
    try {
        con_->Write(request);
        ret = true;
    }
    catch (std::exception& e) {
        std::cout<<" EXCEPTION 0: " << e.what() << std::endl;
        Response fail;
        con_->InterpretResponse(fail);
        std::cout<<"FAIL REPSONSE : " << fail.code << std::endl;
        std::cout<<"FAIL HEADER : " << fail.header.asString() << std::endl;
        std::cout<<"FAIL BODY : " << fail.body << std::endl;
    }
    return ret;
}

bool PlainFileUpload::GenerateDownloadPost(const std::string& filepath, DownloadPost& out) {
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
        out.set_filesize(size_buffer);
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

bool PlainFileUpload::WriteFileToConnection(const std::string& filepath) {
    bool ret = false;
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
            con_->Write(binbuf);
        }
        ifs.close();
        ret = true;
    }
    return ret;
}


} //namespace


