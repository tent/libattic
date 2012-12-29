
#ifndef ROLLSUM_H_
#define ROLLSUM_H_
#pragma once

#include <stdint.h>
#include <iostream>

//#define BLOBBITS (13)
#define BLOBBITS (22)
#define BLOBSIZE (1<<BLOBBITS)
//#define WINDOWBITS (7)
//#define WINDOWSIZE (1<<(WINDOWBITS-1))

#define WINDOWSIZE (64)
#define CHAR_OFFSET 31 


class Rollsum 
{
public:
    Rollsum()
    {
        s1 = WINDOWSIZE * CHAR_OFFSET;
        s2 = WINDOWSIZE * (WINDOWSIZE - 1) * CHAR_OFFSET;
        wofs = 0;
        memset(window, 0, WINDOWSIZE);
    }
 
    ~Rollsum(){}

    void Add(uint8_t drop, uint8_t add)
    {
        s1 += (unsigned int)add - (unsigned int)drop;
        s2 += s1 - ((unsigned int)WINDOWSIZE * (unsigned int)(drop + CHAR_OFFSET));
    }

    void Roll(char ch)
    {
        Add(window[wofs], ch);
        window[wofs] = ch;
        wofs = (wofs +1) % WINDOWSIZE;
    }

    bool OnSplit()
    {
        return (s2 & (BLOBSIZE-1)) == ((~0) & (BLOBSIZE-1));
    }

    uint32_t Digest()
    {
        return (s1 << 16) | (s2 & 0xffff);
    }

    unsigned int GetS1() { return s1; }
    unsigned int GetS2() { return s2; }
    int GetWofs() { return wofs; }

private:
    unsigned int s1;
    unsigned int s2;
    uint8_t window[WINDOWSIZE];
    int wofs; // Window offset
};

static uint32_t rollsum_sum(unsigned char *buf, size_t ofs, size_t len)
{
    size_t count;
    Rollsum r;
    for (count = ofs; count < len; count++)
        r.Roll(buf[count]);
    return r.Digest();
}

static int FindOffset(unsigned char* buf, unsigned int len, int *bits)
{
    Rollsum r;
    int count;
    std::cout<<" Len : " << len << std::endl;
    std::cout<<" Blobsize : " << BLOBSIZE << std::endl;
    std::cout<<" Supposed to be Equal to : " << ((~0) & (BLOBSIZE-1)) << std::endl;

    for (count = 0; count < len; count++)
    {
        r.Roll(buf[count]);
        if (r.OnSplit())
        {
            std::cout<<" SPLIT EQUAL "<<std::endl;
            if (bits)
            {
                unsigned rsum = r.Digest(); 
                *bits = BLOBBITS;
                rsum >>= BLOBBITS;
                for (*bits = BLOBBITS; (rsum >>= 1) & 1; (*bits)++)
                ;
            }
            return count+1;
        }
    }

    std::cout<< " S1 : " << r.GetS1() << std::endl;
    std::cout<< " S2 : " << r.GetS2() << std::endl;
    std::cout<< " Count : " << count << " : " << (r.GetS2() & (BLOBSIZE-1)) << std::endl;
    std::cout<<" returning 0 "<< std::endl;
    return 0;
}

#endif

