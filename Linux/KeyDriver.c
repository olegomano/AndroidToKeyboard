#include <stdio.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>
#include "UsbDevice.h"

void readListener(u_char* data, int lenth);

int main(){
	printf("Hello World\n");
	libusb_context* cntx;
	libusb_init(&cntx);
	libusb_set_debug(cntx, 3); //set verbosity level to 3, as suggested in the documentation
	
	UsbDevice android_device;
	connectToAndroidDevice(cntx,&android_device); //we now have a handle on the device and full controll over the IO 
	
	UsbDeviceReadListener listner;
	listner.dev = &android_device;
	listner.listener = &readListener;

	startListening(&listner);

	static int transferred;
	int error;
	char user_input[1024];

	const char* test_string = "";
	sendData(&android_device,test_string,strlen(test_string));
	usleep(15000);
	freeDevice(&android_device);
	return 0;
}

void readListener(u_char* data, int lenth){
	printf(data);
	printf("\n");
}