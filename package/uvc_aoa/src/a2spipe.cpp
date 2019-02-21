/*
    AOA Proxy - a general purpose Android Open Accessory Protocol host implementation
    Copyright (C) 2012 Tim Otto

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Created on: Oct 21, 2012
 *      Author: Tim
 */

/**
* usb 数据读写
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include "a2spipe.h"
#include "log.h"

#include "v4l2uvc.h"
#include "h264_xu_ctrls.h"
#include "H264_UVC_TestAP.h"


#define FN_SPEW printf
#define FN_ERROR printf
#define FN_WARNING printf
#define FN_FLOOD printf

#define BUFFER_SIZE 511

static void a2s_usbrx_cb(struct libusb_transfer *transfer) {
	usbXferThread *t = (usbXferThread*)transfer->user_data;
	A2spipe::tickleUsbXferThread(t);
}

void A2spipe::tickleUsbXferThread(usbXferThread *t) {
	pthread_mutex_lock( &t->mutex );
	if(t->usbActive) {
		t->usbActive = 0;
		pthread_cond_signal( &t->condition );
	}
	pthread_mutex_unlock( &t->mutex );
}

//usb写入socket任务
void *A2spipe::a2s_usbRxThread( void *d ) {
	logDebug("a2s_usbRxThread started\n");

	struct listentry *device = (struct listentry*)d;

	unsigned char buffer[device->droid.inpacketsize];
	printf("USB rx pack size = %d\n",device->droid.outpacketsize);
	int rxBytes = 0;
	int r;

	//初始化usbRxThread.xfr ，关联数据buffer   传输完毕后回调a2s_usbrx_cb  解锁device->usbRxThread.condition
	libusb_fill_bulk_transfer(device->usbRxThread.xfr, device->droid.usbHandle, device->droid.inendp,
			buffer, sizeof(buffer),
			(libusb_transfer_cb_fn)&a2s_usbrx_cb, (void*)&device->usbRxThread, 0);

	while(!device->usbRxThread.stop && !device->usbDead && !device->TxDead) {

		pthread_mutex_lock( &device->usbRxThread.mutex );
		device->usbRxThread.usbActive = 1;

		//请求数据
		r = libusb_submit_transfer(device->usbRxThread.xfr);
		if (r < 0) {
			logError("a2s usbrx submit transfer failed\n");
			device->usbDead = 1;
			device->usbRxThread.usbActive = 0;
			pthread_mutex_unlock( &device->usbRxThread.mutex );
			break;
		}

		//等待接收数据
		pthread_cond_wait( &device->usbRxThread.condition, &device->usbRxThread.mutex);
		if (device->usbRxThread.usbActive) {
			logError("wait, unlock but usbActive!\n");
		}
		pthread_mutex_unlock( &device->usbRxThread.mutex );

		if (device->usbRxThread.stop || device->usbDead || device->TxDead)
			break;
		
		//查看usb接收数据的状态
		switch(device->usbRxThread.xfr->status) {
		case LIBUSB_TRANSFER_COMPLETED:
		
			rxBytes = device->usbRxThread.xfr->actual_length;
			buffer[rxBytes] = 0;
			printf("%s\n", buffer);
			break;
		case LIBUSB_TRANSFER_NO_DEVICE:
			device->usbDead = 1;
			device->usbRxThread.stop = 1;
			break;
		default:
			break;
		}
		usleep(100000);
			
	}

	device->usbRxThread.stopped = 1;
	logDebug("a2s_usbRxThread finished\n");
	pthread_exit(NULL);
}

extern struct vdIn *vd;
extern struct buffer *buffers;

struct my_video{
	int len;		//长度
	int out;		//以出队
	uint8_t* data;	//数据指针
};

//socket写入usb任务
void *A2spipe::a2s_usbTxThread( void *d ) {
	logDebug("a2s_usbTxThread started\n");

	int ret;
	struct v4l2_buffer buf;
	struct my_video video_buf;
	video_buf.data = NULL;
	video_buf.len = 0;
	video_buf.out = 0;

	struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
	fd_set rfds;
    int retval=0;


	struct listentry *device = (struct listentry*)d;

	unsigned char buffer[device->droid.outpacketsize];
	printf("USB tx pack size = %d\n",device->droid.outpacketsize);
	int rxBytes = 0;
	int r = 0;



	//初始化usbRxThread.xfr ，关联数据buffer
	libusb_fill_bulk_transfer(device->usbTxThread.xfr, 
		device->droid.usbHandle, 
		device->droid.outendp,
		buffer, 
		sizeof(buffer),	
		(libusb_transfer_cb_fn)a2s_usbrx_cb, 
		(void*)&device->usbTxThread, 
		0);

	device->usbTxThread.xfr->status = LIBUSB_TRANSFER_COMPLETED;

	while(!device->usbTxThread.stop && !device->usbDead && !device->TxDead) {
		if(video_buf.len == 0 )
		{
			if(video_buf.data != NULL)free(video_buf.data);

			memset (&buf, 0, sizeof (buf));
				
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;

			FD_ZERO(&rfds);
            FD_SET(vd->fd, &rfds);
            
            retval=select(vd->fd + 1, &rfds, NULL, NULL, &tv);
            if(retval<0)
            {  
                  perror("select error\n");  
            }
            else//有数据要收
            {
			
				ret = ioctl(vd->fd, VIDIOC_DQBUF, &buf);
				if (ret < 0) 
				{
					printf("Unable to dequeue buffer!\n");
					exit(1);
				}

				video_buf.len = buf.bytesused;
				//printf("1 buffer lenght = %d\n",buf.bytesused);
				video_buf.out = 0;	  
				video_buf.data = (uint8_t*)malloc(buf.bytesused);
				memcpy(video_buf.data,(uint8_t*)(buffers[buf.index].start),buf.bytesused);

				ret = ioctl(vd->fd, VIDIOC_QBUF, &buf);
				
				if (ret < 0) 
				{
					printf("Unable to requeue buffer");
					exit(1);
				}
			}
			
		}else{

			if (device->usbTxThread.xfr->status == LIBUSB_TRANSFER_COMPLETED) {
				if(video_buf.len >= BUFFER_SIZE){
					memcpy(buffer,video_buf.data + video_buf.out, BUFFER_SIZE);
					video_buf.out += BUFFER_SIZE;
					video_buf.len -= BUFFER_SIZE;
					rxBytes = BUFFER_SIZE;
				}else{
					memcpy(buffer, video_buf.data + video_buf.out, video_buf.len);
					//printf("2 buffer lenght = %d\n", video_buf.out + video_buf.len);
					rxBytes = video_buf.len;
					video_buf.out = 0;
					video_buf.len = 0;
					free(video_buf.data);
					video_buf.data = NULL;
				}

			}	

			if (device->usbRxThread.stop || device->usbDead || device->TxDead)break;

			pthread_mutex_lock( &device->usbTxThread.mutex );
			device->usbTxThread.usbActive = 1;
			device->usbTxThread.xfr->length = rxBytes;

			//异步发送buffer中的数据到usb
			r = libusb_submit_transfer(device->usbTxThread.xfr);
			if (r < 0) {
				logError("a2s usbtx submit transfer failed\n");
				device->usbDead = 1;
				device->usbTxThread.usbActive = 0;
				pthread_mutex_unlock( &device->usbTxThread.mutex );
				break;
			}

			//等待数据发送完毕
			pthread_cond_wait( &device->usbTxThread.condition, &device->usbTxThread.mutex);
			if (device->usbTxThread.usbActive) {
				logError("wait, unlock but usbActive!\n");
			}
			pthread_mutex_unlock( &device->usbTxThread.mutex );

			//查看数据发送结果
			switch(device->usbTxThread.xfr->status) {
			case LIBUSB_TRANSFER_COMPLETED:
	//			logDebug("USB TX %d/%d bytes DONE\n", device->socketRxThread.xfr->actual_length, rxBytes);
				break;
			case LIBUSB_TRANSFER_NO_DEVICE:
				device->usbDead = 1;
				device->usbRxThread.stop = 1;
				break;
			default:
	//			logDebug("a2s_socketRxThread usb error %d, ignoring\n", device->socketRxThread.xfr->status);
				break;
			}
			//usleep(1);
		}
	}

	device->usbTxThread.stopped = 1;
	logDebug("a2s_socketRxThread finished\n");
	logDebug("closed video file\n");
	pthread_exit(NULL);
}

