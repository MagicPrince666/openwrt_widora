#include "CONSTANT.h"
#include <string.h>
#include <stdio.h>

static int PORT = 554;
static  int BITRATE = 1024*1024;
static  int FPS = 30;
static  char* VIDEODEV = new char[20]{0};


void setPort(int p)
{
    PORT = p;
}

void setBitRate(int b)
{
    BITRATE = b * 1024;
}

void setFps(int fps)
{
    FPS = fps;
}

void setVideoDevice(char* dev)
{
    strcpy(VIDEODEV, dev);
    printf("VD:%s dev:%s\n", VIDEODEV, dev);
}


int getFps()
{
    return  FPS;
}

int getBitRate()
{
    return  BITRATE;
}

int getPort()
{
    return  PORT;
}

char* getVideoDevice()
{
    printf("getVideoDevice: %s \n", VIDEODEV);
    return VIDEODEV;
}
