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
#include "Daemon.h"

//incase of usb.h is missing do the following
// sudo apt-get install libsubs-dev


int alive =  1;

/*
void onDataRecieved(UsbDevice* dev,u_char* data, int length){
	int* recieved_data = (int*)data;
	printf("Key Pressed: %04x \n", *recieved_data);
	char pressedKey;
	if(dev->endianess == SAME){
		pressedKey = data[0];
	}else{
		pressedKey = data[3];
	}
	keyPress(pressedKey);
}; //when data is recieved
*/
void daemonMain(int pid){
	DAEMON_FILE_PRINT("DAEMON STARTED  ", pid);
}



int main(){
	

	openUinput();
	android_device_create_context();
	int dev_id = android_device_reg(4046,20923);
	while(1){
		android_device_poll_events();
	}
	return 0;
}