#include "ringbuffer.h"
#include <string.h>
#include <stdio.h>


#define Min(x, y) ((x) < (y) ? (x) : (y))

cycle_buffer* buffer;
pthread_mutex_t mut;//声明互斥变量

RingBuffer::RingBuffer()
{
    printf("Init cycle buffer\n");
    
    buffer = (cycle_buffer *)malloc(sizeof(cycle_buffer));
    if (!buffer) return; 
    memset(buffer, 0, sizeof(RingBuffer)); 

    buffer->size = DEFAULT_BUF_SIZE;  
    buffer->in   = 0;
    buffer->out  = 0;  

    buffer->buf = (unsigned char *)malloc(buffer->size);  
    if (!buffer->buf)
    {
        free(buffer);
        return;
    }
    memset(buffer->buf, 0, DEFAULT_BUF_SIZE);

    pthread_mutex_init(&mut,NULL);
}

RingBuffer::~RingBuffer()
{
    if(buffer) {
        free(buffer->buf);
        buffer->buf = NULL;
        free(buffer);
        buffer = NULL;
    }
}


int RingBuffer::Reset()
{
    if (buffer == NULL)
    {
        return -1;
    }
     
    buffer->in   = 0;
    buffer->out  = 0;
    memset(buffer->buf, 0, buffer->size);
    
    printf("RingBuffer cleaned\n");

    return 0;
}

int RingBuffer::empty()
{
    return buffer->in == buffer->out;
}

//get buffer size canbe usb
int RingBuffer::overage()
{
    int overage = buffer->in - buffer->out;

    if(overage > 0)
        return DEFAULT_BUF_SIZE - overage;
    else
        return DEFAULT_BUF_SIZE + overage;
}

int RingBuffer::write(uint8_t *data,unsigned int length)
{
    unsigned int len = 0;

    length = Min(length, buffer->size - buffer->in + buffer->out);  
    len    = Min(length, buffer->size - (buffer->in & (buffer->size - 1)));

    pthread_mutex_lock(&mut);
    memcpy(buffer->buf + (buffer->in & (buffer->size - 1)), data, len);
    memcpy(buffer->buf, data + len, length - len);
    pthread_mutex_unlock(&mut);
 
    buffer->in += length;
 
    return length;
}

int RingBuffer::read(uint8_t *target,unsigned int amount)
{
    unsigned int len = 0;  

    amount = Min(amount, buffer->in - buffer->out);
    len    = Min(amount, buffer->size - (buffer->out & (buffer->size - 1)));
 
    pthread_mutex_lock(&mut);
    memcpy(target, buffer->buf + (buffer->out & (buffer->size - 1)), len);
    memcpy(target + len, buffer->buf, amount - len);
    pthread_mutex_unlock(&mut);
 
    buffer->out += amount;
 
    return amount;
}

