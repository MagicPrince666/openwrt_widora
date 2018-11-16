#ifndef __RINGBUFFER_H_
#define __RINGBUFFER_H_

#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>

#define DEFAULT_BUF_SIZE 1*1024*1024

typedef struct 
{  
    uint8_t*  buf;
    unsigned int   size;
    unsigned int   in;
    unsigned int   out;
}cycle_buffer;  


class RingBuffer {
public:
    //cycle_buffer* buffer;
    RingBuffer();
    ~RingBuffer();

    static int read(uint8_t *target,unsigned int amount);
    static int write(uint8_t *data,unsigned int length);
    static int empty();
    static int overage();
    static int Reset();

protected:   
    
    
};

#endif