#include "plainfileupload.h"

namespace attic { 

PlainFileUpload::PlainFileUpload(const std::string& url) {
    url_ = url;
    con_ = NULL;
    utils::GenerateRandomString(boundary_, 20);
}

PlainFileUpload::~PlainFileUpload() {}

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

} //namespace

