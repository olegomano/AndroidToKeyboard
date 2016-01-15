#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#include <stdio.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>


struct UsbDevice{
	int is_valid;
	int is_open;
	int is_accessory;
	int interface;
	uint16_t vendor_id;
	uint16_t product_id; 
	libusb_device* device;
	libusb_device_handle* device_handle;

};
typedef struct UsbDevice UsbDevice;

void listDevices(libusb_context* cntx);
UsbDevice findAndroidDevice(libusb_context* cntx);
int setToAccessoryMode(UsbDevice* device);
UsbDevice connectToAccessory(libusb_context* cntx);
int connectToAndroidDevice(libusb_context* cntx, UsbDevice* device);
void freeDevice(UsbDevice* device);

#endif