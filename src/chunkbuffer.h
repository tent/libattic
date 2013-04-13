#ifndef CHUNKBUFFER_H_
#define CHUNKBUFFER_H_
#pragma once

#include <string>
#include <fstream>
#include <iostream>

#include "constants.h"
#include "rollsum.h"

namespace attic { 

class ChunkBuffer { 
    void ReadToWindow();
public:
    ChunkBuffer(const unsigned int buffer_read = cnst::g_unMaxBuffer,
                const unsigned int max_split = cnst::g_unSplitMax,
                const unsigned int min_split = cnst::g_unSplitMin);
    ~ChunkBuffer();

    bool OpenFile(const std::string& filepath);
    bool ReadChunk(std::string& out);
    bool CloseFile();

    bool BufferEmpty() { if(window_buffer_.size()) return false; return true; }

private:
    RollSum rs_;
    std::ifstream ifs_;
    std::string filepath_;

    std::string window_buffer_;
    std::string remainder_buffer_;

    unsigned int filesize_;
    unsigned int max_buffer_read_; // max amount to read into a buffer at one time
    unsigned int max_split_;
    unsigned int min_split_;

    size_t total_read_;
    size_t pos_;
};

} //namespace
#endif

