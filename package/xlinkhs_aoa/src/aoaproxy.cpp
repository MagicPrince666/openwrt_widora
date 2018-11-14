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
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "aoaproxy.h"
#include "accessory.h"
#include "a2spipe.h"
#include "log.h"

//控制主循环
static int do_exit = 0;
static libusb_context *ctx;
//维护当前所有usb与对应socket的链表
struct listentry *connectedDevices;
//pthread_t usbInventoryThread;
int doUpdateUsbInventory = 0;

static int startUSBPipe(struct listentry *device);
static void stopUSBPipe(struct listentry *device);

static struct t_excludeList *exclude = NULL;
static int autoscan = 1;

/*
static void a2s_usbrx_cb(struct libusb_transfer *transfer) {
	usbXferThread *t = (usbXferThread*)transfer->user_data;
	A2spipe::tickleUsbXferThread(t);
}
*/
int main(int argc, char** argv) {
	int r;
	//int numBytes = 0;
	unsigned char *buff = new unsigned char[1024];
	memset(buff,0,1024);
	sprintf((char *)buff,"magic prince");

	ctx = NULL;
	connectedDevices = NULL;

	AoaProxy::initSigHandler();

	if (0 > AoaProxy::initUsb()) {
		logError("Failed to initialize USB\n");
		return 1;
	}

	if(autoscan) {
		struct itimerval timer;
		timer.it_value.tv_sec = 1;
		timer.it_value.tv_usec = 0;
		timer.it_interval.tv_sec = 1;
		timer.it_interval.tv_usec = 0;
		setitimer (ITIMER_REAL, &timer, NULL);
	}

	libusb_device **devs = NULL;
	while(!do_exit) {

		if (doUpdateUsbInventory == 1) {
			doUpdateUsbInventory = 0;
			AoaProxy::cleanupDeadDevices();
			//尝试链接设备
			AoaProxy::updateUsbInventory(devs);
		
		}

		r = libusb_handle_events(ctx);
		if (r) {
			if (r == LIBUSB_ERROR_INTERRUPTED) {
				// ignore
			} else {
				if(!do_exit)
					logDebug("libusb_handle_events_timeout: %d\n", r);

				break;
			}
		}
	}

	if (devs != NULL)
		libusb_free_device_list(devs, 1);

	if(autoscan) {
		struct itimerval timer;
		memset (&timer, 0, sizeof(timer));
		setitimer (ITIMER_REAL, &timer, NULL);
	}
	AoaProxy::shutdownEverything();

	delete[] buff;
	return EXIT_SUCCESS;
}

void AoaProxy::shutdownEverything() {
	logDebug("shutdownEverything\n");
	do_exit = 1;

	struct itimerval timer;
	memset (&timer, 0, sizeof(timer));
	setitimer (ITIMER_REAL, &timer, NULL);

	while(connectedDevices != NULL)
		disconnectDevice(connectedDevices->usbDevice);

	if (ctx != NULL)
		libusb_exit(ctx); //close the session

	logDebug("shutdown complete\n");
}

void AoaProxy::tickleUsbInventoryThread() {
	doUpdateUsbInventory = 1;
}

int AoaProxy::updateUsbInventory(libusb_device **devs) {
	static ssize_t cnt = 0;
	static ssize_t lastCnt = 0;
//	static libusb_device **devs;
	static libusb_device **lastDevs = NULL;
	//获取usb设备列表
	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0) {
		logError("Failed to list devices\n");
		return -1;
	}

	ssize_t i, j;
	int foundBefore;
	for(i = 0; i < cnt; i++) {
		foundBefore = 0;
		if ( lastDevs != NULL) {
			for(j=0;j < lastCnt; j++) {
				if (devs[i] == lastDevs[j]) {
					foundBefore = 1;
					break;
				}
			}
		}
		if (!foundBefore) {
			logDebug("start connect device\n");
			//连接设备 连接本地服务端
			if(connectDevice(devs[i]) >= 0)
				libusb_ref_device(devs[i]);
		}
	}

	if (lastDevs != NULL) {
//		if (cnt != lastCnt)
//			fprintf(LOG_DEB, "number of USB devices changed from %d to %d\n", lastCnt, cnt);

		for (i=0;i<lastCnt;i++) {
			foundBefore = 0;
			for(j=0;j<cnt;j++) {
				if (devs[j] == lastDevs[i]) {
					foundBefore = 1;
					break;
				}
			}
			if(!foundBefore) {
				struct listentry *hit = connectedDevices;

				while(hit != NULL) {
					if ( hit->usbDevice == lastDevs[i]) {
						disconnectDevice(lastDevs[i]);
						libusb_unref_device(lastDevs[i]);
						break;
					}
					hit = hit->next;
				}
			}
		}
		libusb_free_device_list(lastDevs, 1);
	}
	lastDevs = devs;
	lastCnt = cnt;
	
	return 0;
}

int AoaProxy::connectDevice(libusb_device *device) {

	logDebug("prepare to connect device \n");

	struct libusb_device_descriptor desc;
	//获取usb设备描述信息
	int r = libusb_get_device_descriptor(device, &desc);
	if (r < 0) {
		logError("failed to get device descriptor: %d\n", r);
		return -1;
	}

	switch(desc.bDeviceClass) {
	case 0x09:
		logDebug("device 0x%04X:%04X has wrong deviceClass: 0x%02x\n",
				desc.idVendor, desc.idProduct,
				desc.bDeviceClass);
		return -1;
	}

	struct t_excludeList *e = exclude;
	while(e != NULL) {
		logDebug("comparing device [%04x:%04x] and [%04x:%04x]\n",
				desc.idVendor, desc.idProduct, e->vid, e->pid);
		if(e->vid == desc.idVendor && e->pid == desc.idProduct) {
			logDebug("device is on exclude list idVendor = %d idProduct = %d\n", desc.idVendor, desc.idProduct);
			return -1;
		}
		e = e->next;
	}

	//检查当前设备是否处于accessory模式
	if(!Accessory::isDroidInAcc(device)) {
		logDebug("attempting AOA on device 0x%04X:%04X\n",
				desc.idVendor, desc.idProduct);
		//写入要启动的应用的信息 开启android的accessory模式	
		Accessory::switchDroidToAcc(device, 1);
		return -1;
	}

	//entry管理socket与usb
	struct listentry *entry = (struct listentry *)malloc(sizeof(struct listentry));
	if (entry == NULL) {
		logError("Not enough RAM\n");
		return -2;
	}
	bzero(entry, sizeof(struct listentry));

	//entry拿到usb句柄device
	entry->usbDevice = device;

	//如果android设备已经是aoa模式,打开usb
	logDebug("start setup droid \n");
	//找到accessory接口并用接口信息初始化entry->droid
	r = Accessory::setupDroid(device, &entry->droid);
	if (r < 0) {
		logError("failed to setup droid: %d\n", r);
		free(entry);
		return -3;
	}

	//将entry加入链表
	entry->next = NULL;
	if (connectedDevices == NULL) {
		entry->prev = NULL;
		connectedDevices = entry;
	} else {
		struct listentry *last = connectedDevices;
		while(last->next != NULL)
			last = last->next;
		entry->prev = last;
		last->next = entry;
	}

	//建立usb与socket互相通信的任务
	r = startUSBPipe(entry);
	if (r < 0) {
		logError("failed to start pipe: %d\n", r);
		disconnectDevice(device);
		return -5;
	}

	logDebug("new Android connected\n");
	return 0;
}

void AoaProxy::disconnectDevice(libusb_device *dev) {
	struct listentry *device = connectedDevices;

	while(device != NULL) {
		if (device->usbDevice == dev) {
			if (device->prev == NULL) {
				connectedDevices = device->next;
			} else {
				device->prev->next = device->next;
			}
			if (device->next != NULL) {
				device->next->prev = device->prev;
			}

			stopUSBPipe(device);
			Accessory::shutdownUSBDroid(device->usbDevice, &device->droid);

			free(device);
			logDebug("Android disconnected\n");
			return;
		}
		device = device->next;
	}
}

void AoaProxy::cleanupDeadDevices() {
	struct listentry *device = connectedDevices;

	while(device != NULL) {
		if (device->usbDead) {
			logDebug("found device with dead USB\n");
		} else {
			device = device->next;
			continue;
		}
		disconnectDevice(device->usbDevice);
		cleanupDeadDevices();
		break;
	}
}


int initUsbXferThread(usbXferThread *t) {
//	t->dead = 0;
	t->xfr = libusb_alloc_transfer(0);
	if (t->xfr == NULL) {
		return -1;
	}
	t->stop = 0;
	t->stopped = 0;
	t->usbActive = 0;
	t->tickle = 0;
	pthread_mutex_init(&t->mutex, NULL);
	pthread_cond_init(&t->condition, NULL);
	return 0;
}

void destroyUsbXferThread(usbXferThread *t) {
	pthread_mutex_destroy(&t->mutex);
	pthread_cond_destroy(&t->condition);
	libusb_free_transfer(t->xfr);
}

int startUSBPipe(struct listentry *device) {
	int r;
	if(initUsbXferThread(&device->usbRxThread) < 0) {
		logError("failed to allocate usb rx transfer\n");
		return -1;
	}
	if(initUsbXferThread(&device->usbTxThread) < 0) {
		logError("failed to allocate usb tx transfer\n");
		destroyUsbXferThread(&device->usbRxThread);
		return -1;
	}

	//写入到usb任务
	r = pthread_create(&device->usbRxThread.thread, NULL, A2spipe::a2s_usbRxThread, (void*)device);
	if (r < 0) {
		logError("failed to start usb rx thread\n");
		return -1;
	}

	//读出到socket任务
	r = pthread_create(&device->usbTxThread.thread, NULL, A2spipe::a2s_usbTxThread, (void*)device);
	if (r < 0) {
		// other thread is stopped in disconnectDevice method
		logError("failed to start usbTxThread thread\n");
		return -1;
	}

	return 0;
}

void stopUSBPipe(struct listentry *device) {

	device->usbRxThread.stop = 1;
	if(device->usbRxThread.usbActive) {
		libusb_cancel_transfer(device->usbRxThread.xfr);
	}
	A2spipe::tickleUsbXferThread(&device->usbRxThread);
	pthread_kill(device->usbRxThread.thread, SIGUSR1);
	logDebug("waiting for usb rx thread...\n");
	if(0 != pthread_join(device->usbRxThread.thread, NULL)) {
		logError("failed to join usb rx thread\n");
	}

	device->usbTxThread.stop = 1;
	if(device->usbTxThread.usbActive) {
		libusb_cancel_transfer(device->usbTxThread.xfr);
	}
	A2spipe::tickleUsbXferThread(&device->usbTxThread);
	pthread_kill(device->usbTxThread.thread, SIGUSR1);
	logDebug("waiting for usbTxThread thread...\n");
	if(0 != pthread_join(device->usbTxThread.thread, NULL)) {
		logError("failed to join usbTxThread thread\n");
	}

	destroyUsbXferThread(&device->usbRxThread);
	destroyUsbXferThread(&device->usbTxThread);

	logDebug("threads stopped\n");
}
int audioError = 1;

void AoaProxy::initSigHandler() {
	struct sigaction sigact;
	sigact.sa_handler = sig_hdlr;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGUSR1, &sigact, NULL);
	sigaction(SIGUSR2, &sigact, NULL);
	sigaction(SIGVTALRM, &sigact, NULL);
	sigaction(SIGALRM, &sigact, NULL);
}

void AoaProxy::sig_hdlr(int signum)
{
	switch (signum) {
	case SIGINT:
		logDebug("received SIGINT\n");
		do_exit = 1;
		exit(1);
		break;
	case SIGUSR1:
		logDebug("received SIGUSR1\n");
		break;
	case SIGUSR2:
		tickleUsbInventoryThread();
		break;
	case SIGALRM:
	case SIGVTALRM:
		tickleUsbInventoryThread();
		break;
	default:
		break;
	}
}

int AoaProxy::initUsb() {

	int r;
	r = libusb_init(&ctx);
	if(r < 0) {
		return r;
	}
	libusb_set_debug(ctx, 3);
	return 0;
}

