#include <stdio.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>
#include "UsbDevice.h"

#define IN 0x81
#define OUT 0x02

int main(){
	printf("Hello World\n");
	libusb_context* cntx;
	libusb_init(&cntx);
	libusb_set_debug(cntx, 3); //set verbosity level to 3, as suggested in the documentation
	
	UsbDevice android_device;
	connectToAndroidDevice(cntx,&android_device); //we now have a handle on the device and full controll over the IO 
	//freeDevice(&android_device);
	unsigned char buffer[16384] = "testTESTtestTEST";
	static int transferred;
	while(1){
		libusb_clear_halt(android_device.device_handle,OUT);
		int response = libusb_bulk_transfer(android_device.device_handle, OUT, buffer, 16, &transferred, 900);
		switch(response){
			case 0: break;
			case LIBUSB_ERROR_TIMEOUT: printf("The transfer timed out (populates transferred) \n"); break; 
    		case LIBUSB_ERROR_PIPE: printf("the endpoint halted \n"); break;
    		case LIBUSB_ERROR_OVERFLOW: printf("Overflow, the device offered more data\n"); break;
    		case LIBUSB_ERROR_NO_DEVICE: printf("The device has been disconnected \n"); break;
    		default: printf("Undefined error\n"); 
		}
		printf("I have transferred %d bytes \n", transferred );
		usleep(350);
 
	}
	return 0;
}