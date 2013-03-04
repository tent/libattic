#ifndef ROLLSUM_H_
#define ROLLSUM_H_
#pragma once

#include <iostream>
#include <string.h>
#include <stdint.h>

static const int windowSize = 64;
static const int charOffset = 31;

class RollSum
{
public:
    RollSum(int bits = 22)
    {   
        m_Wofs = 0;
        m_S1 = windowSize * charOffset; 
        m_S2 = windowSize * (windowSize -1) * charOffset;
        memset(m_Window, 0, windowSize);

        m_BlobBits = bits;
        m_BlobSize = 1 << m_BlobBits;   // 1 << 22 ~ 4mbs
    }
    
    ~RollSum() {}
    
    void add(uint32_t drop, uint32_t add)
    {   
        m_S1 += add - drop;
        m_S2 += m_S1 - uint32_t(windowSize) * uint32_t(drop + uint32_t(charOffset));
    }
    
    void Roll(char byte)
    {   
        add(m_Window[m_Wofs], byte);
        m_Window[m_Wofs] = byte;
        m_Wofs = (m_Wofs + 1) % windowSize;
    }
    
    bool split(int size)
    {
        if((m_S2 & (size - 1)) == ((0^0) & (size - 1)))
            return true;
        return false;
    }
    bool OnSplit()
    {   
        int min = 1 << 21;
        int max = 1 << 23;

        //if((m_S2 & (m_BlobSize - 1)) == ((0^0) & (m_BlobSize - 1)))
        if(split(max) || split(min))
            return true;
        return false;
    }

    int GetSize() const { return m_BlobSize; } 

private:
    uint32_t    m_S1;
    uint32_t    m_S2;
    uint8_t     m_Window[windowSize];
    int         m_Wofs;
    int         m_BlobBits;
    int         m_BlobSize;
};

namespace roll
{

    // Example usage
    static void CalculateSplits(const std::string& data)
    {
        RollSum s;
        for(uint32_t k = 0; k<data.size();++k) {
            char c = data[k];
            s.Roll(c);
            if(s.OnSplit())
                std::cout<< " split : " << k << std::endl;
        }
    }

    static void CaclulateSplitsForFile(const std::string& filepath, std::deque<unsigned int> out)
    {


    }

}
#endif

