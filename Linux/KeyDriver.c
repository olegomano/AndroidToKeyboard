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

UsbDeviceStatusListener device;
int alive =  1;

void daemonMain(int pid){
	DAEMON_FILE_PRINT("DAEMON STARTED  ", pid);
	//mainOld();
}

void onDeviceConnected(){
	printf("Connected to device\n");
}; //when usb is physically plugged in

void onDeviceDisconnected(){
	printf("Device is disconnected\n");
	device.device.is_read = 0;
}; //when usb is physically disconnected

void onDeviceClosed(){

}; //when logical usb connection is closed

void onDeivceOpened(){
	registerForDataRead(&device);
}; //when logical usb connection is establised 

void onLibUsbFail(char* errmsg, int errcode){
	printf("USB ERROR: %s, %d \n", errmsg,errcode);
};

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

void libusb_poll_thread(){
	while(alive){
		libusb_handle_events_completed(NULL, NULL);
	}
}

int main(){
	libusb_context* cntx;
	libusb_init(&cntx);
	libusb_set_debug(cntx, 3); //set verbosity level to 3, as suggested in the documentation
	openUinput();

	device.onLibUsbFail = &onLibUsbFail;
	device.onDataRecieved = &onDataRecieved;
	device.onDeivceOpened = &onDeivceOpened;
	device.onDeviceClosed = &onDeviceClosed;
	device.onDeviceConnected = &onDeviceConnected;
	device.onDeviceDisconnected = &onDeviceDisconnected;
	connectToAndroidDeviceHotplug(&device,cntx,4046,20923);
	int* zeroArr = malloc(128);// this is a dirty hack, I couldn't find the defenition struct timeval
								// but by the defenition if it has all 0's it means its non blocking
								//so i hope that the struct is less than 128 bytes in size, and set 128 bytes to 0
	memset(zeroArr,0,128);
	while(1){
		libusb_handle_events_timeout(cntx,(struct timeval*)zeroArr);
	}
	delete(zeroArr);
	alive = 0;
	return 0;
}