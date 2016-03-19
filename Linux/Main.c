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

#include <sys/stat.h>
#include <fcntl.h> 
#include <stdio.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "UinputWrapper.h"
#include "UsbDevice.h"
#include "FrameBuffer.h"
#include "X11Wrapper.h"

#define MODE_KBM 1
#define KEY_EVENT 1
#define MOUSE_MOVE_EVENT 2
#define MOUSE_CLICK_EVENT 3
#define MOUSE_SCROLL_EVENT 4

#define MODE_SS 2
//incase of usb.h is missing do the following
// sudo apt-get install libsubs-dev


int flipInt(int wrong){
	int right;
	char* wrong_char = (char*)(&wrong);
	char* right_char = (char*)(&right);
	right_char[0] = wrong_char[3];
	right_char[1] = wrong_char[2];
	right_char[2] = wrong_char[1];
	right_char[3] = wrong_char[0];

	return right;
}
/*
	packets are 32 bytes
	0xFFFF | 0xFFFF 0xFFFF 0xFFFF 0xFFFF 0xFFFF 0xFFFF 0xFFFF 
	 type     mode    dx    dy
     data         pressedKey
*/
     /*
void onDataRead(int dev_id, char* data){
	int* recieved_data = (int*)data;
	printf("Key Pressed: %04x \n", *(recieved_data + 1) );
	char pressedKey; //ascii code of requested key
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
			uinput_key_press(pressedKey);
			break;
		case MOUSE_MOVE_EVENT:
			uinput_mouse_move(dx,dy);
			break;
		case MOUSE_CLICK_EVENT:
			printf("Click\n");
			uinput_mouse_click();
			break;
		case MOUSE_SCROLL_EVENT:
			printf("Scroll %d\n",dx );
			uinput_mouse_scroll(dx);
			break;	
	}
};
*/
void kbmMode(int dev_id, char* data){
	printf("Mouse Mode\n");
	int* data_int = (int*)(data);
	printf("%08x, %08x %08x\n", data_int[0],data_int[1],data_int[2]);
	int mode;
	int first_arg;
	int second_arg;
	if(android_device_get_device_id(dev_id)->endianess == SAME){
		mode = data_int[0];
		first_arg = data_int[1];
		second_arg = data_int[2];
	}else{
		mode = flipInt(data_int[0]);
		first_arg = flipInt(data_int[1]);
		second_arg = flipInt(data_int[2]);
	}
	printf("Mouse Type %d\n", mode);
	switch(mode){
		case KEY_EVENT:
			uinput_key_press(first_arg);
			break;
		case MOUSE_MOVE_EVENT:
			uinput_mouse_move(first_arg,second_arg);
			break;
		case MOUSE_CLICK_EVENT:
			uinput_mouse_click();	
			break;
		case MOUSE_SCROLL_EVENT:
			uinput_mouse_scroll(first_arg);
			break;
	}

}

void ssMode(int dev_id, char* data){

}


void onDataRead(int dev_id, char* data){
	char mode = data[0];
	printf("Mode: %d\n", mode);
	switch(mode){
		case MODE_KBM: kbmMode(dev_id,data + 3); break;
		case MODE_SS:  ssMode(dev_id,data + 3); break;
	}
}

void onAndroidConnected(int dev_id){

};

void onAndroidDisConnected(int dev_id){

};

int time_function(int count, void (*funct)() ){
	struct timeval start, end;
    long mtime, seconds, useconds;    
    gettimeofday(&start, NULL);
    int av_count;
    
    for(av_count = 0; av_count < count; av_count++)
   		funct();
    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    return mtime/count;
}

pthread_t android_thread_h;
volatile int android_thread_status = -1;
void android_thread(){
	while(android_thread_status){
		android_device_poll_events();
	}
}

pthread_t x11_thread_h;
volatile int x11_thread_status = -1;
void x11_thread(){
	while(x11_thread_status){

	}

}

int main(){
	uinput_open();
	android_device_create_context();
	AndroidDeviceCallbacks callbacks;
	callbacks.onDataRead = onDataRead;
	callbacks.onAndroidConnected = onAndroidConnected;
	callbacks.onAndroidDisConnected  = onAndroidDisConnected;
	
	int dev_id = android_device_reg(4046,20923);
	android_device_set_callbacks(dev_id,callbacks);
    x11_init();
    x11_print();
    x11_thread_status = 1;
    android_thread_status = 1;
    pthread_create(&x11_thread_h, NULL, &x11_thread, NULL);
	pthread_create(&android_thread_h, NULL, &android_thread, NULL);
	char* command;
	while(1){
		
	}
	return 0;
}