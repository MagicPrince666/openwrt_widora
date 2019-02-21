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

#include <stdlib.h>
#include <string.h>

#include "accessory.h"
#include "log.h"

const char *vendor = "Huang Liquan";
const char *model = "android.usbaoa";
const char *description = "Android AOA Interface";
const char *version = "1.3";
const char *uri = "http://prince.yicp.top";
const char *serial = "xag3a01";

int Accessory::setupDroid(libusb_device *usbDevice, accessory_droid *device) {

	bzero(device, sizeof(accessory_droid));

	struct libusb_device_descriptor desc;
	//获取usb描述符
	int r = libusb_get_device_descriptor(usbDevice, &desc);
	if (r < 0) {
		logError("failed to get device descriptor\n");
		return r;
	}

	struct libusb_config_descriptor *config;
	//获取usb设备配置描述符
	r = libusb_get_config_descriptor(usbDevice, 0, &config);
	if (r < 0) {
		logError("failed t oget config descriptor\n");
		return r;
	}

	const struct libusb_interface *inter;
	const struct libusb_interface_descriptor *interdesc;
	const struct libusb_endpoint_descriptor *epdesc;
	
	int i,j,k;

	for(i=0; i<(int)config->bNumInterfaces; i++) {
		logDebug("checking interface #%d\n", i);
		inter = &config->interface[i];
		if (inter == NULL) {
			logDebug("interface is null\n");
			continue;
		}
		logDebug("interface has %d alternate settings\n", inter->num_altsetting);
		for(j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			if (interdesc->bNumEndpoints == 2
					&& interdesc->bInterfaceClass == 0xff
					&& interdesc->bInterfaceSubClass == 0xff &&
					(device->inendp <= 0 || device->outendp <= 0)) {

				//找到当前为accessory的接口
				logDebug( "interface %d is accessory candidate\n", i);
				for(k=0; k < (int)interdesc->bNumEndpoints; k++) {
					epdesc = &interdesc->endpoint[k];

					if (epdesc->bmAttributes != 0x02) {
						break;
					}
					//epdesc->wMaxPacketSize = 1024;

					//获取usb输入输出endpoint
					if ((epdesc->bEndpointAddress & LIBUSB_ENDPOINT_IN) && device->inendp <= 0) {
						device->inendp = epdesc->bEndpointAddress;
						device->inpacketsize = epdesc->wMaxPacketSize;
						logDebug( "using EP 0x%02x as bulk-in EP\n", (int)device->inendp);
						logDebug( "inpacketsize = %d\n", epdesc->wMaxPacketSize);
					} else if ((!(epdesc->bEndpointAddress & LIBUSB_ENDPOINT_IN)) && device->outendp <= 0) {
						device->outendp = epdesc->bEndpointAddress;
						device->outpacketsize = epdesc->wMaxPacketSize;
						logDebug( "using EP 0x%02x as bulk-out EP\n", (int)device->outendp);
						logDebug( "outpacketsize = %d\n", epdesc->wMaxPacketSize);
					} else {
						break;
					}
				}
				if (device->inendp && device->outendp) {
					device->bulkInterface = interdesc->bInterfaceNumber;
				}
			} 
		}
	}

	if (!(device->inendp && device->outendp)) {
		logError("device has no accessory endpoints\n");
		return -2;
	}

	//打开usb设备
	r = libusb_open(usbDevice, &device->usbHandle);
	if(r < 0) {
		logError("failed to open usb handle\n");
		return r;
	}

	//设置批量传输 
	r = libusb_claim_interface(device->usbHandle, device->bulkInterface);
	if (r < 0) {
		logError("failed to claim bulk interface\n");
		libusb_close(device->usbHandle);
		return r;
	}

	return 0;
}

int Accessory::shutdownUSBDroid(libusb_device *usbDevice, accessory_droid *device) {

	if ((device->inendp && device->outendp))
		libusb_release_interface(device->usbHandle, device->bulkInterface);

	if(device->usbHandle != NULL)
		libusb_close(device->usbHandle);

	return 0;
}

int Accessory::isDroidInAcc(libusb_device *dev) {
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		logError("failed to get device descriptor\n");
//		fprintf(LOG_ERR, ERR_USB_DEVDESC);
		return 0;
	}

	if (desc.idVendor == VID_GOOGLE) {
		switch(desc.idProduct) {
		case PID_AOA_ACC:
		case PID_AOA_ACC_ADB:
		case PID_AOA_ACC_AU:
		case PID_AOA_ACC_AU_ADB:
			return 1;
		case PID_AOA_AU:
		case PID_AOA_AU_ADB:
			logDebug("device is audio-only\n");
			break;
		default:
			break;
		}
	}

	return 0;
}

//用于切换android设备到acc模式 
void Accessory::switchDroidToAcc(libusb_device *dev, int force) {
	struct libusb_device_handle* handle;
	int r;

	if(0 > libusb_open(dev, &handle)){
		logError("Failed to connect to device\n");
		return;
	}

	if(libusb_kernel_driver_active(handle, 0) > 0) {
		if(!force) {
			logError("kernel driver active, ignoring device");
			libusb_close(handle);
			return;
		}
		if(libusb_detach_kernel_driver(handle, 0)!=0) {
			logError("failed to detach kernel driver, ignoring device");
			libusb_close(handle);
			return;
		}
	}

	const char *setupStrings[6];
	setupStrings[0] = vendor;
	setupStrings[1] = model;
	setupStrings[2] = description;
	setupStrings[3] = version;
	setupStrings[4] = uri;
	setupStrings[5] = serial;

	int i;
	for(i=0;i<6;i++) {
		if(0 > (r = libusb_control_transfer(handle,
				0x40,
				52,
				0,
				(uint16_t)i,
				(unsigned char*)setupStrings[i],
				strlen(setupStrings[i]),2000))) {
			logDebug( "send string %d call failed\n", i);
			libusb_close(handle);
			return;
		}
	}

	if(0 > (r = libusb_control_transfer(handle,0x40,53,0,0,NULL,0,2000))) {
		logDebug( "start accessory call failed\n");
		libusb_close(handle);
		return;
	}

	libusb_close(handle);
}

