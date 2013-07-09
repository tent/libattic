#include "chunkbuffer.h"

#include <iostream>

namespace attic { 

ChunkBuffer::ChunkBuffer(const unsigned int buffer_read,
                         const unsigned int max_split,
                         const unsigned int min_split) {
    max_buffer_read_ = buffer_read;
    max_split_ = max_split;
    min_split_ = min_split;
    total_read_ = 0;
    pos_= 0;
}

ChunkBuffer::~ChunkBuffer() {}

bool ChunkBuffer::OpenFile(const std::string& filepath) {
    ifs_.open(filepath.c_str(), std::ios::in | std::ios::binary);
    if(ifs_.is_open()) {
        ifs_.seekg(0, std::ifstream::end);
        filesize_ = ifs_.tellg();
        ifs_.seekg(0, std::ifstream::beg);
        ReadToWindow();
    }
    return ifs_.is_open();
}

bool ChunkBuffer::CloseFile() { 
    ifs_.close();
    return !ifs_.is_open();
}
void ChunkBuffer::ReadToWindow() { 
    // Determine how much to read in
    char* data = NULL;
    unsigned int data_size = 0;
    if(total_read_ < filesize_) {
        if((filesize_ - total_read_) >= max_buffer_read_) {
            // If the difference is greater than max buffer, read max buffer
            data = new char[max_buffer_read_];
            data_size = max_buffer_read_;
        }
        else {
            unsigned int read_diff = filesize_ - total_read_;
            if(read_diff <= max_buffer_read_) {
                data = new char[read_diff];
                data_size = read_diff;
            }
            else {
                data = new char[max_buffer_read_];
                data_size = max_buffer_read_;
            }
        }

        // clear the window, and append data
        ifs_.read(data, data_size);
        total_read_ += data_size;
        if(filesize_ < total_read_)
            std::cout<<" WE READ MORE THAN THE FILESIZE? " << std::endl;
    }

    if(data_size)
        window_buffer_.append(data, data_size);

    if(data) { 
        delete[] data;
        data = NULL;
    }
}

bool ChunkBuffer::ReadChunk(std::string& out) {
    bool chunked = false;

    for(unsigned int i=0; i<window_buffer_.size(); i++) {
        // Roll each byte
        char c = window_buffer_[i];
        rs_.Roll(c);

        if(rs_.OnSplit() && (i >= min_split_)) {
            // Split the window buffer
            out = window_buffer_.substr(0, i);
            window_buffer_.erase(0, i);
            chunked = true;
            break;
        }

        // Check for max split, and force
        if(i >= max_split_) { 
            out = window_buffer_.substr(0, i);
            window_buffer_.erase(0, i);
            chunked = true;
            break;
        }
    }

    if(!chunked) {
        // Load more into the buffer and try again...
        if(!EndOfFile()) {
            ReadToWindow();
            chunked = ReadChunk(out);
        }
        else if(window_buffer_.size()) {
            out = window_buffer_;
            window_buffer_.clear();
            chunked = true;
        }
    }

    return chunked;
}

bool ChunkBuffer::EndOfFile() {
    if(total_read_ >= filesize_)
        return true;
    return false;
}

} //namespace
