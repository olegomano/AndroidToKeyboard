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

#define READ_PACKET_SIZE 32

#define CONNECTION_STATUS_CONNECTED 1
#define CONNECTION_STATUS_DISCONNECTED 0

#define TRASNFER_STATUS_HANDSHAKE 0
#define TRANSFER_STATUS_READING 1
#define TRANSFER_STATUS_WAITING 2    


struct AndroidDeviceCallbacks{
	void (*onDataRead)(int dev_id, char* data);
	void (*onControlMessage)(int dev_id, char* data);
	void (*onAndroidConnected)(int dev_id);
	void (*onAndroidDisConnected)(int dev_id);
	void (*onAndroidTransferStateChanged)(int dev_id, int new_state);
};
typedef struct AndroidDeviceCallbacks AndroidDeviceCallbacks;

struct AndroidDevice{
	uint16_t vendor_id;
	uint16_t product_id; 
	
	volatile int dev_id;
	volatile int conncetion_status; //connecetd / disconnected
	volatile int transfer_status; // handshake, reading, waiting
	volatile int packet_size;
	volatile int endianess;

	volatile uint8_t port_numbers[7];
	libusb_device* device;              
	libusb_device_handle* device_handle;

	AndroidDeviceCallbacks callback;
};
typedef struct AndroidDevice AndroidDevice;

int android_device_create_context();
int android_device_reg(int vid, int pid); //returns ID of device
int android_device_destroy_context();
int android_device_poll_events();
int android_device_send_data(int dev_id, unsigned char* data, int length);
int android_device_set_callbacks(int dev_id, AndroidDeviceCallbacks callback);
void android_device_print_device(int dev_id);
AndroidDevice* android_device_get_device(int vid, int pid);
AndroidDevice* android_device_get_device_id(int dev_id);

#endif