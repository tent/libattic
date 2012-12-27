
#ifndef ROLLSUM_H_
#define ROLLSUM_H_
#pragma once

#include <stdint.h>

#define BLOBBITS (13)
//#define BLOBBITS (20)
#define BLOBSIZE (1<<BLOBBITS)
#define WINDOWBITS (7)
#define WINDOWSIZE (1<<(WINDOWBITS-1))
#define ROLLSUM_CHAR_OFFSET 31 


class Rollsum 
{
public:
    Rollsum() {} 
    ~Rollsum(){}
    
    void Init()
    {
        s1 = WINDOWSIZE * ROLLSUM_CHAR_OFFSET;
        s2 = WINDOWSIZE * (WINDOWSIZE - 1) * ROLLSUM_CHAR_OFFSET;
        wofs = 0;
        memset(window, 0, WINDOWSIZE);
    }

    unsigned int s1;
    unsigned int s2;
    uint8_t window[WINDOWSIZE];
    int wofs; // Window offset
};

#define rollsum_roll(r, ch) do { \
    rollsum_add((r), (r)->window[(r)->wofs], (ch)); \
    (r)->window[(r)->wofs] = (ch); \
    (r)->wofs = ((r)->wofs + 1) % WINDOWSIZE; \
} while (0)

static void rollsum_add(Rollsum *r, uint8_t drop, uint8_t add)
{
    r->s1 += add - drop;
    r->s2 += r->s1 - (WINDOWSIZE * (drop + ROLLSUM_CHAR_OFFSET));
}

static uint32_t rollsum_digest(Rollsum *r)
{
        return (r->s1 << 16) | (r->s2 & 0xffff);
}


static uint32_t rollsum_sum(unsigned char *buf, size_t ofs, size_t len)
{
    size_t count;
    Rollsum r;
    r.Init();
    for (count = ofs; count < len; count++)
        rollsum_roll(&r, buf[count]);
    return rollsum_digest(&r);
}

static int FindOffset(unsigned char* buf, unsigned int len, int *bits)
{
    Rollsum r;
    int count;
    r.Init();

    for (count = 0; count < len; count++)
    {
        rollsum_roll(&r, buf[count]);
        if ((r.s2 & (BLOBSIZE-1)) == ((~0) & (BLOBSIZE-1)))
        {
            if (bits)
            {
                unsigned rsum = rollsum_digest(&r);
                *bits = BLOBBITS;
                rsum >>= BLOBBITS;
                for (*bits = BLOBBITS; (rsum >>= 1) & 1; (*bits)++)
                ;
            }
            return count+1;
        }
    }
    return 0;

}


#endif

