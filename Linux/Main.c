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
#include <math.h>
#include <unistd.h>

#include "UinputWrapper.h"
#include "UsbDevice.h"
#include "FrameBuffer.h"
#include "X11Wrapper.h"

#define BUFFER_FREE 0
#define BUFFER_USED 1

#define MODE_KBM 1
#define KEY_EVENT 1
#define MOUSE_MOVE_EVENT 2
#define MOUSE_CLICK_EVENT 3
#define MOUSE_SCROLL_EVENT 4

#define MODE_SS 2
#define REQUEST_FB_DIMS 1
#define REQUEST_SS_START 2
#define REQEUST_SS_STOP 3   
#define SS_DATA 4 
#define SS_SYN 5

#define SS_PACKET_HEADER 8

typedef struct {
	int fb_size;
	int total_packets;
	int data_per_packet;
	int leftover_bytes;
	int scale;
} ScreenGrabParams;

char* x11_buffer[3];
int   x11_buffer_used[3];

//incase of usb.h is missing do the following
// sudo apt-get install libsubs-dev
int time_function(int count, void (*funct)(int i) );

long int get_millis(){
	struct timeval tp;
	gettimeofday(&tp,NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	return ms;
}

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

void sendScreenCapture(int dev_id, ScreenGrabParams* params){
	long get_frame_start = get_millis();
	x11FBuffer* screen_buffer = x11_getframe();
	long get_frame_end = get_millis();
	printf("Get Frame Time %f\n", (get_frame_end - get_frame_start)/1000.0f);

	AndroidDevice* device = android_device_get_device_id(dev_id);
	int packet_count = params->total_packets;
	int data_per_packet = params->data_per_packet;
	
	int current_packet;
	int screen_buffer_pos = 0;
	int packet_pos = 0;

	long int send_start = get_millis();
	for(current_packet = 0; current_packet < packet_count -1; current_packet++){
		screen_buffer_pos = data_per_packet*current_packet;
		packet_pos = current_packet * (data_per_packet + SS_PACKET_HEADER);
		device->buffer[packet_pos    ] = MODE_SS;
		device->buffer[packet_pos + 1] = SS_DATA;
		*((int*)(&device->buffer[packet_pos + 4])) = current_packet;
		memcpy(device->buffer + packet_pos + SS_PACKET_HEADER,(char*)screen_buffer->fb_data + screen_buffer_pos,data_per_packet);
	}
	screen_buffer_pos = data_per_packet*current_packet;
	packet_pos = current_packet * (data_per_packet + SS_PACKET_HEADER);
	device->buffer[packet_pos    ] = MODE_SS;
	device->buffer[packet_pos + 1] = SS_DATA;
	*((int*)(&device->buffer[packet_pos + 4])) = current_packet;
	memcpy(device->buffer + packet_pos + SS_PACKET_HEADER,(char*)screen_buffer->fb_data + screen_buffer_pos,params->leftover_bytes);

	android_device_send_data_buffer(dev_id,device->buffer_size,320);
	long int send_end = get_millis();
	printf("Packet Send Time: %f \n", ( (float)(send_end - send_start) / 1000.0f) );

	long int syn_start = get_millis();
	device->buffer[0] = MODE_SS;
	device->buffer[1] = SS_SYN;
	android_device_send_data_buffer(dev_id,device->packet_size,0);
	long int syn_end = get_millis();
	printf("Syn Time: %f \n", ( (float)(syn_end - syn_start) / 1000.0f) );

}	

void sendBufferedScreenCap(char* buffer, int dev_id, ScreenGrabParams* params){
	AndroidDevice* device = android_device_get_device_id(dev_id);
	int packet_count = params->total_packets;
	int data_per_packet = params->data_per_packet;
	
	int current_packet;
	int screen_buffer_pos = 0;
	int packet_pos = 0;

	long int send_start = get_millis();
	for(current_packet = 0; current_packet < packet_count -1; current_packet++){
		screen_buffer_pos = data_per_packet*current_packet;
		packet_pos = current_packet * (data_per_packet + SS_PACKET_HEADER);
		device->buffer[packet_pos    ] = MODE_SS;
		device->buffer[packet_pos + 1] = SS_DATA;
		*((int*)(&device->buffer[packet_pos + 4])) = current_packet;
		memcpy(device->buffer + packet_pos + SS_PACKET_HEADER,buffer + screen_buffer_pos,data_per_packet);
	}
	screen_buffer_pos = data_per_packet*current_packet;
	packet_pos = current_packet * (data_per_packet + SS_PACKET_HEADER);
	device->buffer[packet_pos    ] = MODE_SS;
	device->buffer[packet_pos + 1] = SS_DATA;
	*((int*)(&device->buffer[packet_pos + 4])) = current_packet;
	memcpy(device->buffer + packet_pos + SS_PACKET_HEADER,buffer + screen_buffer_pos,params->leftover_bytes);

	android_device_send_data_buffer(dev_id,device->buffer_size,320);
	long int send_end = get_millis();
	printf("Packet Send Time: %f \n", ( (float)(send_end - send_start) / 1000.0f) );

	long int syn_start = get_millis();
	device->buffer[0] = MODE_SS;
	device->buffer[1] = SS_SYN;
	android_device_send_data_buffer(dev_id,device->packet_size,0);
	long int syn_end = get_millis();
	printf("Syn Time: %f \n", ( (float)(syn_end - syn_start) / 1000.0f) );
}

void ssMode(int dev_id, char* data){
	AndroidDevice* device = android_device_get_device_id(dev_id);
	char* device_buffer = device->buffer;
	int*  device_buffer_int = (int*)device->buffer;

	int* data_int = (int*)data;
	int request;
	int first_arg;
	int second_arg;
	int third_arg;
	int f_arg;
	if(android_device_get_device_id(dev_id)->endianess == SAME){
		request = data_int[0];
		first_arg = data_int[1];
		second_arg = data_int[2];
		third_arg = data_int[3];
		f_arg = data[4];
	}else{
		request = flipInt(data_int[0]);
		first_arg = flipInt(data_int[1]);
		second_arg = flipInt(data_int[2]);
		third_arg = flipInt(data_int[3]);
		f_arg = flipInt(data_int[4]);
	}
	printf("Flag: %d\n", request);
	switch(request){
		case REQUEST_FB_DIMS:
			printf("Request for FB dims\n");
			device_buffer[0] = MODE_SS;
			device_buffer[1] = REQUEST_FB_DIMS;
			device_buffer[2] = 15;
			device_buffer[3] = 15;
			
			printf("Framebuffer w,h: %d %d \n",x11_get_fb_w(),x11_get_fb_h() );
			int frame_size = device->packet_size - SS_PACKET_HEADER;
		    int total_size = x11_get_fb_w() * x11_get_fb_h() * 4.0;
			int total_frames =  ceil( total_size / (double) frame_size ); 
			int leftover_bytes = total_size - frame_size*(total_frames - 1);			

			device_buffer_int[1] = x11_get_fb_w();
			device_buffer_int[2] = x11_get_fb_h();
			device_buffer_int[3] = total_frames;
			android_device_send_data_buffer(dev_id,device->packet_size,0);
			free(device->buffer);
			device->buffer        = malloc(total_frames*device->packet_size);
			device->buffer_size   = total_frames*device->packet_size;
			
			device->extra_data[0]    = malloc(sizeof(ScreenGrabParams));
			ScreenGrabParams* params = device->extra_data[0];
			params->fb_size          = x11_get_fb_w() * x11_get_fb_h() * 4;
			params->scale            = f_arg;
			params->total_packets    = total_frames;
			params->data_per_packet  = frame_size;
			params->leftover_bytes   = leftover_bytes;
			
			if(x11_buffer[0] != NULL){
				free(x11_buffer[0]);
				free(x11_buffer[1]);
				free(x11_buffer[2]);
			}
			x11_buffer[0] = malloc(x11_get_fb_h()*x11_get_fb_w()*4);
     		x11_buffer[1] = malloc(x11_get_fb_h()*x11_get_fb_w()*4);
     		x11_buffer[2] = malloc(x11_get_fb_h()*x11_get_fb_w()*4);
     		//while(1) sendScreenCapture(dev_id,params);
			break;

		case REQUEST_SS_START: 
			device->transfer_status = TRANSFER_STATUS_SCREEN_SHARE; 
			printf("Request to Start SS\n");
			x11_thread_start(dev_id);
			break;
		case REQEUST_SS_STOP:  
			device->transfer_status = TRANSFER_STATUS_WAITING; 
			printf("Request to End SS\n");
			x11_thread_stop();
			break;
	}
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
pthread_t usb_send_thread_h;
volatile int x11_thread_status = 0;
volatile int x11_thread_kill   = 0;
volatile int x11_buffer_index  = 0;
volatile int x11_buffer_new    = 0;
pthread_mutex_t x11_thread_mutex;
pthread_mutex_t x11_status_mutex;
pthread_mutex_t x11_buffer_mutex;



void x11_thread(int dev_id){
	printf("Thread Started for Dev %d \n", dev_id);
	pthread_mutex_lock(&x11_thread_mutex);
	x11_thread_status = 1;
	ScreenGrabParams* params = android_device_get_device_id(dev_id)->extra_data[0];	
	int mIndex = 0;
	while(1){
		pthread_mutex_lock(&x11_status_mutex);
		if(x11_thread_kill) {
			printf("x11 read kill flag\n");
			pthread_mutex_unlock(&x11_status_mutex);
			break;
		}
		pthread_mutex_unlock(&x11_status_mutex);
		
		pthread_mutex_lock(&x11_buffer_mutex);
		x11_buffer_new++;
		for(mIndex = 0; mIndex < 3; mIndex++){
			if(x11_buffer_used[mIndex] == BUFFER_FREE){
				mIndex = BUFFER_FREE;
				break;
			}
		}
		x11_buffer_used[mIndex] = BUFFER_USED;
		pthread_mutex_unlock(&x11_buffer_mutex);
		
		x11FBuffer* screen_buffer = x11_getframe();
		memcpy(x11_buffer[mIndex],screen_buffer->fb_data,params->fb_size);

		pthread_mutex_lock(&x11_buffer_mutex);
		x11_buffer_used[mIndex] = BUFFER_FREE;
		x11_buffer_index = mIndex;
		pthread_mutex_unlock(&x11_buffer_mutex);	
	}
	pthread_mutex_unlock(&x11_thread_mutex);
	printf("Thread Ended for Dev %d \n", dev_id);
}

void usb_send_thread(int dev_id){
	int mIndex;
	ScreenGrabParams* params = android_device_get_device_id(dev_id)->extra_data[0];	
	while(1){
		pthread_mutex_lock(&x11_status_mutex);
		if(x11_thread_kill) {
			printf("x11 read kill flag\n");
			pthread_mutex_unlock(&x11_status_mutex);
			break;
		}
		pthread_mutex_unlock(&x11_status_mutex);
		
		pthread_mutex_lock(&x11_buffer_mutex);
		if(x11_buffer_new > 0){
			printf("Skipped frames %d \n", (x11_buffer_new - 1) );
			x11_buffer_new = 0;
			mIndex = x11_buffer_index;
			x11_buffer_used[mIndex] = BUFFER_USED;
			pthread_mutex_unlock(&x11_buffer_mutex);
			
			sendBufferedScreenCap(x11_buffer[mIndex],dev_id,params);
			
			pthread_mutex_lock(&x11_buffer_mutex);
			x11_buffer_used[mIndex] = BUFFER_FREE;
		}
		pthread_mutex_unlock(&x11_buffer_mutex);


	}
}


void x11_thread_start(int dev_id){
	pthread_mutex_lock(&x11_status_mutex);
	if(x11_thread_status && !x11_thread_kill){
		pthread_mutex_unlock(&x11_status_mutex);
		return;
	}
	pthread_mutex_unlock(&x11_status_mutex);

	pthread_mutex_lock(&x11_thread_mutex);
	
	pthread_mutex_lock(&x11_status_mutex);
	x11_thread_status = 1;
	x11_thread_kill = 0;
	pthread_mutex_unlock(&x11_status_mutex);
	pthread_create(&x11_thread_h, NULL, &x11_thread, dev_id);
	pthread_create(&usb_send_thread_h,NULL,&usb_send_thread,dev_id);
	pthread_mutex_unlock(&x11_thread_mutex);    	
}

void x11_thread_stop(){
	pthread_mutex_lock(&x11_status_mutex);
	x11_thread_kill = 1;
	printf("x11 raised kill flag\n");
	pthread_mutex_unlock(&x11_status_mutex);
}

int main(){
	
	x11_buffer[0] = NULL;
	x11_buffer[1] = NULL;
	x11_buffer[2] = NULL;
	x11_buffer_used[0] = BUFFER_FREE;
	x11_buffer_used[1] = BUFFER_FREE;
	x11_buffer_used[2] = BUFFER_FREE;
	x11_buffer_new = 0;

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
    android_thread_status = 1;
    pthread_create(&android_thread_h, NULL, &android_thread, NULL);
	char* command;
	while(1){
		
	}
	return 0;
}