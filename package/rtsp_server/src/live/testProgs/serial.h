#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <termios.h>
#include <errno.h>   
#include <limits.h> 
#include <asm/ioctls.h>
#include <time.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/epoll.h> 
#include <string.h>

#define MAXEVENT 15

#define DATA_LEN                0xFF
#define CDC_SIZE 1024         

class Serial
{

public:
    int fd[2];//文件描述符
    int run;
    //int pipe_fd;//有名管道句柄  
    int epid; //epoll标识符

    int ComRead(char * ReadBuff,const int ReadLen);//com口读数据
    int EpollInit(int *cfd);
    int openSerial(const char *cSerialName);

private:  
    struct epoll_event event;
    struct epoll_event events[MAXEVENT];//事件集合
};

#endif