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
#define WAITING_FOR_HANDSHAKE 1
#define READY 2

struct UsbDevice{
	volatile int state; 
	volatile int is_valid;
	volatile int packet_size;
	volatile int endianess; //0 is opposite, 1 is same
	int is_open;
	int is_accessory;
	int interface;
	uint16_t vendor_id;
	uint16_t product_id; 
	libusb_device* device;              
	libusb_device_handle* device_handle;
};
typedef struct UsbDevice UsbDevice;

struct UsbDeviceReadListener{
	UsbDevice* dev;
	void (*listener)(u_char* data, int length);
};
typedef struct UsbDeviceReadListener UsbDeviceReadListener;

struct UsbDeviceStatusListener{
	UsbDevice* device;
	void (*onDeviceClosed)();
	void (*onDeivceOpened)();
	void (*onDataRecieved)(u_char* data, int length);
};
typedef struct UsbDeviceStatusListener UsbDeviceStatusListener;

void loadAndroidVPIDList(FILE* f);

void printDevice(UsbDevice* dev);
void listDevices(libusb_context* cntx);

int  connectToAndroidDevice(libusb_context* cntx, UsbDevice* device);
int openDevice(libusb_context* cntx, UsbDevice* device, UsbDeviceStatusListener* listener);
void freeDevice(UsbDevice* device);


int  sendData(UsbDevice* device, void* data, int length);
void sendDataAsync(UsbDevice* device, void* data, int length);

pthread_t startListening(UsbDeviceReadListener* listener);
#endif