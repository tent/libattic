#include "chunkrequest.h"

#include "utils.h"
#include "netlib.h"
#include "errorcodes.h"
#include "event.h"
#include "jsonserializable.h"

namespace attic { 

ChunkRequest::ChunkRequest(const std::string& entity,
                           const std::string& posts_feed, 
                           const std::string& post_path, 
                           const std::string& meta_post_id,
                           const AccessToken& at,
                           const unsigned int group_number) {
    request_type_ = "POST"; // default
    entity_ = entity;
    access_token_ = at;
    posts_feed_ = posts_feed;
    post_path_ = post_path;
    meta_post_id_ = meta_post_id;
    url_ = posts_feed;
    has_parent_ = false;
    socket_ = NULL;
    group_number_ = group_number;
    post_.set_group(group_number);
}

ChunkRequest::~ChunkRequest() {}

void ChunkRequest::set_parent_post(const ChunkPost& cp) { 
    parent_post_ = cp; 
    has_parent_ = true;
    url_.clear();
    utils::FindAndReplace(post_path_, "{post}", parent_post_.id(), url_);
    request_type_ = "PUT";
}

void ChunkRequest::BeginRequest() {
    if(request_type_ == "PUT") {
        Parent parent;
        parent.version = parent_post_.version()->id;
        post_.PushBackParent(parent);
    }

    std::string protocol, host, path;
    netlib::ExtractHostAndPath(url_, protocol, host, path);
    std::cout<<" Begining Chunk Request " << std::endl;
    std::cout<<" protocol : " << protocol << std::endl;
    std::cout<<" host : " << host << std::endl;
    std::cout<<" path : " << path << std::endl;
     
    socket_ = new Connection(&io_service_);
    socket_->Initialize(url_);

    utils::GenerateRandomString(boundary_, 20);

    // Build header request
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    netlib::BuildRequestHeader(request_type_, url_, boundary_, &access_token_, request_stream); 
        
    // Start the request
    socket_->Write(request);
}

int ChunkRequest::PushBackChunk(const ChunkInfo& ci,
                                const std::string& chunk_name, 
                                const std::string& chunk,
                                const unsigned int count) { 
    int status = ret::A_OK;

    post_.PushBackChunkInfo(ci, count);

    // Check parent if chunk exists
    if(has_parent_ && parent_post_.HasChunk(chunk_name)) {
        std::cout<<" Parent already has chunk " << std::endl;
        // copy over attachment
        if(parent_post_.has_attachment(chunk_name)) {
            Attachment attachment = parent_post_.get_attachment(chunk_name);
            post_.PushBackAttachment(attachment);
        }
        else { 
            std::cout<<" PARENT POST DOESN'T HAVE ATTACHMENT?!?!? " << std::endl;
        }
    }
    else {
        // Build Attachment and write to socket
        boost::asio::streambuf attachment;
        std::ostream attachmentstream(&attachment);
        netlib::BuildAttachmentForm(chunk_name, chunk, boundary_, count, attachmentstream);

        boost::asio::streambuf part;
        std::ostream chunkpartbuf(&part);
        netlib::ChunkPart(attachment, chunkpartbuf);
        status = WriteToSocket(part);
    }

    return status;
}

void ChunkRequest::EndRequest(Response& out) {
    // Send chunk post body
    if(!entity_.empty() && !meta_post_id_.empty()) {
        post_.MentionPost(entity_, meta_post_id_);
    }
    else {
        std::cout<<" SOMETHING IS EMPTY " << std::endl;
        std::cout<<" ENTITY : " << entity_ << std::endl;
        std::cout<<" META POST ID : " << meta_post_id_ << std::endl;
    }

    std::string body;
    jsn::SerializeObject(&post_, body);

    boost::asio::streambuf requestBody;
    std::ostream part_stream(&requestBody);
    netlib::BuildBodyForm(post_.type(), body, boundary_, part_stream);
    netlib::AddEndBoundry(part_stream, boundary_);

    // Chunk the Body 
    boost::asio::streambuf chunkedBody;
    std::ostream partbuf(&chunkedBody);

    std::cout<<" END REQUEST BODY : " << body << std::endl;

    netlib::ChunkEnd(requestBody, partbuf);
    socket_->Write(chunkedBody);

    std::cout<<" interpreting response " << std::endl;
    socket_->InterpretResponse(out);

    if(socket_) {
        delete socket_;
        socket_ = NULL;
    }
}

int ChunkRequest::WriteToSocket(boost::asio::streambuf& buffer) {
    int breakcount = 0;
    int retrycount = 20;

    int status = ret::A_OK;
    unsigned int buffersize = buffer.size();
    for(unsigned int i=0; i < retrycount; i++) {
        try {
            status = ret::A_OK;
            boost::timer::cpu_timer::cpu_timer t;
            //size_t byteswritten = boost::asio::write(ssl_sock, buffer, errorcode); 
            size_t byteswritten = socket_->Write(buffer);
            std::cout<<" bytes written : " << byteswritten << std::endl;
            boost::timer::cpu_times time = t.elapsed();
            boost::timer::nanosecond_type elapsed = (time.system + time.user);

            // To seconds
            elapsed *= 0.0000000001; 
            std::cout<<" ELAPSED : " << elapsed << std::endl;
            if(elapsed > 0) {
                unsigned int bps = (buffersize/elapsed);
                // Raise event
                char szSpeed[256] = {'\0'};
                snprintf(szSpeed, 256, "%u", bps);
                event::RaiseEvent(event::Event::UPLOAD_SPEED, std::string(szSpeed), NULL);
            }
            break;
        }
        catch(boost::system::system_error& err) {
            std::cout<<" Write Socket error : " << err.what() << std::endl;
            status = ret::A_FAIL_SOCKET_WRITE;
        }
    }

    return status;
}



} // namespace

