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
void daemonMain(int pid){
	DAEMON_FILE_PRINT("DAEMON STARTED  ", pid);
}

void onDataRead(int dev_id, char* data){
	int* recieved_data = (int*)data;
	printf("Key Pressed: %04x \n", *recieved_data);
	char pressedKey;
	if(android_device_get_device_id(dev_id)->endianess == SAME){
		pressedKey = data[0];
	}else{
		pressedKey = data[3];
	}
	keyPress(pressedKey);
};

void onAndroidConnected(int dev_id){};
void onAndroidDisConnected(int dev_id){};
void onAndroidTransferStateChanged(int dev_id, int new_state){};



int main(){
	

	openUinput();
	android_device_create_context();
	AndroidDeviceCallbacks callbacks;
	callbacks.onDataRead = onDataRead;
	callbacks.onAndroidConnected = onAndroidConnected;
	callbacks.onAndroidDisConnected  = onAndroidDisConnected;
	callbacks.onAndroidTransferStateChanged = onAndroidTransferStateChanged;

	int dev_id = android_device_reg(4046,20923);
	android_device_set_callbacks(dev_id,callbacks);



	while(1){
		android_device_poll_events();
	}
	return 0;
}