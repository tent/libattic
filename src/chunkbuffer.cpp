#include "chunkbuffer.h"

namespace attic { 

ChunkBuffer::ChunkBuffer(const unsigned int buffer_read,
                         const unsigned int max_split,
                         const unsigned int min_split) {
    max_buffer_read_ = buffer_read;
    max_split_ = max_split_;
    min_split_ = min_split_;
    total_read_ = 0;
    pos_= 0;
}

ChunkBuffer::~ChunkBuffer() {}

bool ChunkBuffer::OpenFile(const std::string& filepath) {
    ifs_.open(filepath.c_str(), std::ios::in | std::ios::binary);
    if(ifs_.is_open()) {
        ifs_.seekg(0, std::ios::end);
        filesize_ = ifs_.tellg();
        ifs_.seekg(0, std::ios::beg);
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
    if(filesize_ >= max_buffer_read_) {
        data = new char[filesize_];
        data_size = filesize_;
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
    while(!ifs_.eof()) { // while to accumulate
        ReadToWindow();

        int session_read = 0, byte_count = 0, last_split = 0;
        for(unsigned int i=0; i<window_buffer_.size(); i++) {
            // Roll each byte
            char c = window_buffer_[i];
            rs_.Roll(c);
            byte_count++;

            if(rs_.OnSplit() && (byte_count >= min_split_)) {
                std::string chunk;
                int diff = i - last_split;
                // Split the window buffer
                chunk = window_buffer_.substr(last_split, diff);

                last_split = i;
                byte_count = 0;
                session_read += chunk.size();
            }

            // Check for max split, and force
            if(byte_count >= max_split_) { 

            }
        }

        // Set remainder

    }
}

} //namespace
