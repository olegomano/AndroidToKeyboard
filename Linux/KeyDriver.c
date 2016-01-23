#include <stdio.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>
#include "Keyboard.h"
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

	int error;
	const char* test_string = "Now, when I say that I am in the habit of going to sea whenever I begin to grow hazy about the eyes, and begin to be over conscious of my lungs, I do not mean to have it inferred that I ever go to sea as a passenger. For to go as a passenger you must needs have a purse, and a purse is but a rag unless you have something in it. ";
	sendData(&android_device,test_string,strlen(test_string));

	error = openUinput();
	if(error){
		printf("Error opening Uinput\n");
	}

	char user_input[1024];
	while(1){
		memset(user_input,0,1024);
		//scanf("%s",&user_input);
		sendKeyDown('a');
		//sendData(&android_device,user_input,1024);
		usleep(15000);
		if(user_input[0] == 'e' && user_input[1] == 'x' && user_input[2] == 'i' && user_input[3] == 't'){
			break;
		}
	}
	closeUinput();
	freeDevice(&android_device);
	usleep(1000000);
	return 0;
}

void readListener(u_char* data, int lenth){
	int read_int = *((int*)data);
	printf("%x\n",read_int);
}