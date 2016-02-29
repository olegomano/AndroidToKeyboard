/*
	This file is part of KeyboardToLinux.

    KeyboardToLinux is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KeyboardToLinux is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KeyboardToLinux.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#include <stdio.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>
#include <string.h> 

#define IN 0x81
#define OUT 0x02
#define OPPOSITE 0
#define SAME 1

#define DISCONNECTED 0
#define WAITING_FOR_HANDSHAKE 1
#define WORKING 2

struct UsbDevice{
	volatile int is_valid; //is this a valid device
	volatile int is_open; //does we have controll over the interface
	volatile int is_accessory; //are we an accessory
	volatile int is_read; //are we currently reading 

	volatile int status; //logical state of connection
	volatile int packet_size;
	volatile int endianess; //0 is opposite, 1 is same

	int interface;
	uint16_t vendor_id;
	uint16_t product_id; 
	libusb_hotplug_callback_handle hotplug_callback_handle;
	libusb_device* device;              
	libusb_device_handle* device_handle;
};
typedef struct UsbDevice UsbDevice;

struct UsbDeviceStatusListener{
	UsbDevice device;
	void (*onDeviceConnected)(); //when usb is physically plugged in
	void (*onDeviceDisconnected)(); //when usb is physically disconnected
	void (*onDeviceClosed)(); //when logical usb connection is closed
	void (*onDeivceOpened)(); //when logical usb connection is establised 
	void (*onLibUsbFail)(char* errsmg, int errcode);
	void (*onDataRecieved)(UsbDevice* dev,u_char* data, int length); //when data is recieved
};
typedef struct UsbDeviceStatusListener UsbDeviceStatusListener;

void printDevice(UsbDevice* dev);
void listDevices(libusb_context* cntx);

int  connectToAndroidDeviceHotplug(UsbDeviceStatusListener* callback, libusb_context* cntx,int vid, int pid);
int  openDevice(libusb_context* cntx, UsbDevice* device, UsbDeviceStatusListener* listener);
void freeDevice(UsbDevice* device);

int  sendData(UsbDevice* device, void* data, int length);
pthread_t registerForDataRead(UsbDeviceStatusListener* listener);
#endif