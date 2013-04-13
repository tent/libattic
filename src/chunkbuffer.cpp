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
    std::cout<< " FILE SIZE : " << filesize_ << std::endl;
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
    if(!ifs_.eof()) {
        if(filesize_ >= max_buffer_read_) {
            data = new char[max_buffer_read_];
            data_size = max_buffer_read_;
        }
        else {
            unsigned int read_diff = filesize_ - total_read_;
            std::cout<<" READ DIFF : " << read_diff << std::endl;
            std::cout<<" total read : " << total_read_ << std::endl;
            std::cout<<" filesize : " << filesize_ << std::endl;
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
    }

    std::cout<< " data size " << data_size << std::endl;
    window_buffer_.clear();
    window_buffer_ += remainder_buffer_;
    remainder_buffer_.clear();

    if(data_size)
        window_buffer_.append(data, data_size);

    if(data) { 
        delete[] data;
        data = NULL;
    }
}

bool ChunkBuffer::ReadChunk(std::string& out) {
    bool chunked = false;
    if(window_buffer_.size() <= 0);
        ReadToWindow();

    int session_read = 0, byte_count = 0, last_split = 0;
    for(unsigned int i=0; i<window_buffer_.size(); i++) {
        // Roll each byte
        char c = window_buffer_[i];
        rs_.Roll(c);
        byte_count++;

        if(rs_.OnSplit() && (byte_count >= min_split_)) {
            int diff = i - last_split;
            // Split the window buffer
            out = window_buffer_.substr(last_split, diff);

            last_split = i;
            byte_count = 0;
            session_read += out.size();
            chunked = true;
            break;
        }

        // Check for max split, and force
        if(byte_count >= max_split_) { 
            int diff = i - last_split;
            out = window_buffer_.substr(last_split, diff);

            last_split = i;
            byte_count = 0;
            session_read += out.size();
            chunked = true;
            break;
        }
    }

    // Set remainder
    int wdiff = window_buffer_.size() - session_read;
    if(wdiff) {
        remainder_buffer_ = window_buffer_.substr(last_split, wdiff);
        window_buffer_.clear();
        if(chunked) { 
            std::cout<<" chunked lets break " << std::endl;
        }
    }
    std::cout<<" total read so far : " << total_read_ << std::endl;
    std::cout<<" exiting ... " << std::endl;
    return chunked;
}

} //namespace
