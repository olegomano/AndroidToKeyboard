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

#define KEY_EVENT 1
#define MOUSE_MOVE_EVENT 2
#define MOUSE_CLICK_EVENT 3
//incase of usb.h is missing do the following
// sudo apt-get install libsubs-dev


int alive =  1;
void daemonMain(int pid){
	DAEMON_FILE_PRINT("DAEMON STARTED  ", pid);
}

void onDataRead(int dev_id, char* data){
	int* recieved_data = (int*)data;
	printf("Key Pressed: %04x \n", *(recieved_data + 1) );
	char pressedKey;
	int dx;
	int dy;
	char mode;
	if(android_device_get_device_id(dev_id)->endianess == SAME){
		mode = data[0];
		pressedKey = data[4];
		dx = recieved_data[1];
		dy = recieved_data[2];
	}else{
		mode = data[3];
		pressedKey = data[7];
		char* dx_char = &dx;
		char* dy_char = &dy;

		dx_char[0] = data[7];
		dx_char[1] = data[6];
		dx_char[2] = data[5];
		dx_char[3] = data[4];

		dy_char[0] = data[11];
		dy_char[1] = data[10];
		dy_char[2] = data[9];
		dy_char[3] = data[8];

	}
	switch(mode){
		case KEY_EVENT:
			keyPress(pressedKey);
			break;
		case MOUSE_MOVE_EVENT:
			sendMouse(dx,dy);
			break;
		case MOUSE_CLICK_EVENT:
		break;
	}
	
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