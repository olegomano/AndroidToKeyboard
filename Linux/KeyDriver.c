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
#include <stdio.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>
#include "Keyboard.h"
#include "UsbDevice.h"

//incase of usb.h is missing do the following
// sudo apt-get install libsubs-dev

UsbDeviceReadListener listner;

void readListener(u_char* data, int lenth);
int main(){
	printf("Hello World\n");
	libusb_context* cntx;
	libusb_init(&cntx);
	libusb_set_debug(cntx, 3); //set verbosity level to 3, as suggested in the documentation
	
	UsbDevice android_device;
	connectToAndroidDevice(cntx,&android_device); //we now have a handle on the device and full controll over the IO 
	
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
		scanf("%s",&user_input);
		sendData(&android_device,user_input,android_device.packet_size);
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
	int key_press;
	if(listner.dev->endianess == SAME){
		key_press = *((int*)data);
	}else{
		u_char* as_chars = &key_press;
		as_chars[0] = data[3];
		as_chars[1] = data[2];
		as_chars[2] = data[1];
		as_chars[3] = data[0];
	}
	printf("Key Pressed %d\n",key_press);
	sendKeyDown(key_press);
	sendKeyUp(key_press);
	printf("\n\n");
}