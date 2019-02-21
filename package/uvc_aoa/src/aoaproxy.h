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

#ifndef AOAPROXY_H_
#define AOAPROXY_H_

#include <libusb-1.0/libusb.h>
#include "accessory.h"

typedef struct t_excludeList {
	int vid;
	int pid;
	struct t_excludeList *next;
} excludeList_Type;

typedef struct t_usbXferThread {
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	struct libusb_transfer *xfr;
	int usbActive;
	int stop;
	int stopped;
	int tickle;
} usbXferThread;


typedef struct listentry {
	libusb_device *usbDevice; //usb设备
	int TxDead;
	int usbDead;

	accessory_droid droid;  //accessory信息

	usbXferThread usbRxThread;      //写入usb线程
	usbXferThread usbTxThread;   //读取usb线程

	struct listentry *prev;
	struct listentry *next;
} t_listentry;

class AoaProxy
{
public:
	static void shutdownEverything();
	static void initSigHandler();
	static int initUsb();
	static void cleanupDeadDevices();
	static int updateUsbInventory(libusb_device **devs);
	static int connectDevice(libusb_device *device);
	static void disconnectDevice(libusb_device *dev);
	static void sig_hdlr(int signum);
	static void tickleUsbInventoryThread();
};

#endif /* AOAPROXY_H_ */
