#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/mman.h>
#include <thread>
#include <chrono>

using namespace std;

int main(void){
    int fd;
    if((fd = open("/dev/video0", O_RDWR)) < 0){
        perror("open");
        exit(1);
    }

    struct v4l2_capability cap;
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
        perror("VIDIOC_QUERYCAP");
        exit(1);
    }
    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
        fprintf(stderr, "The device does not handle single-planarvideo capture.\n");
        exit(1);
    }

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 1280;
    format.fmt.pix.height = 720;
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("VIDIOC_S_FMT");
        exit(1);
    }

    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        exit(1);
    }

    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
    if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
        perror("VIDIOC_QUERYBUF");
        exit(1);
    }

    void* buffer_start = mmap(
    NULL,
    bufferinfo.length,
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    fd,
    bufferinfo.m.offset);

    if(buffer_start == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    memset(buffer_start, 0, bufferinfo.length);

    struct v4l2_buffer bufferinfo2;
    memset(&bufferinfo2, 0, sizeof(bufferinfo2));
    bufferinfo2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo2.memory = V4L2_MEMORY_MMAP;
    bufferinfo2.index = 0;
    int type = bufferinfo2.type;

    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("VIDIOC_STREAMON");
        exit(1);
    }

    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo2) < 0){
        perror("VIDIOC_QBUF");
        exit(1);
    }

    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo2) < 0){
        perror("VIDIOC_QBUF");
        exit(1);
    }

    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    int jpgfile;
    time_t timer;
    timer = time(NULL);
    char str[260] = {0};
    sprintf(str,"%ld.jpeg",timer);

    if((jpgfile = open(str, O_WRONLY | O_CREAT, 0660)) < 0){
        perror("open");
        exit(1);
    }
    write(jpgfile, buffer_start, bufferinfo2.length);
    close(jpgfile);

    //this_thread::sleep_for(chrono::seconds(6));

    close(fd);
    return EXIT_SUCCESS;
}