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
	int total_packets;
	int data_per_packet;
	int leftover_bytes;
	int scale;
} ScreenGrabParams;

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
	x11FBuffer* screen_buffer = x11_getframe();
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

	android_device_send_data_buffer(dev_id,device->buffer_size,0);
	long int send_end = get_millis();
	printf("Packet Send Time: %f \n", ( (float)(send_end - send_start) / 1000.0f) );

	device->buffer[0] = MODE_SS;
	device->buffer[1] = SS_SYN;
	android_device_send_data_buffer(dev_id,device->packet_size,0);
}	

void sendScreenGrab(int dev_id){
	AndroidDevice* device = android_device_get_device_id(dev_id);
	char* device_buffer = device->buffer;
	int*  device_buffer_int = (int*)device->buffer;
	long int start_screen_capture = get_millis();
	x11FBuffer* screen_capture = x11_getframe();
	printf("Screen Capture takes: %ld \n",  (get_millis() - start_screen_capture));
	device_buffer[0] = MODE_SS;
	device_buffer[1] = SS_DATA;
	
	int frame = 0;
	int frame_size = device->packet_size - sizeof(int)*2;
	int total_size = x11_get_fb_w() * x11_get_fb_h() * 4.0;
	int total_frames =  ceil( total_size / (double) frame_size ); 
	int leftover_bytes = total_size - (total_frames -1)*frame_size;
	int total_size_buffer = total_frames*device->packet_size;

	char* screen_capture_fb = (char*)screen_capture->fb_data;
	long int start_sending_packets = get_millis();
	int current_frame;
	int screen_capture_start;
	int device_buffer_start;
	for(current_frame = 0; current_frame < total_frames - 1; current_frame++){
		device_buffer_start = current_frame*device->packet_size;
		screen_capture_start = current_frame*frame_size;
		device_buffer[device_buffer_start] = MODE_SS;
		device_buffer[device_buffer_start + 1] = SS_DATA;
		*((int*)(&device_buffer[device_buffer_start + 4])) = current_frame; 
		memcpy(device_buffer + device_buffer_start + SS_PACKET_HEADER,screen_capture_fb+screen_capture_start,frame_size); 
	}
	
	device_buffer_start = current_frame*device->packet_size;
	screen_capture_start = current_frame*frame_size;
	device_buffer[device_buffer_start] = MODE_SS;
	device_buffer[device_buffer_start + 1] = SS_DATA;
	*((int*)(&device_buffer[device_buffer_start + 4])) = current_frame; 
	memcpy(device_buffer + device_buffer_start + SS_PACKET_HEADER,screen_capture_fb+screen_capture_start,leftover_bytes); 
	android_device_send_data_buffer(dev_id,total_size_buffer,0);
	printf("Packet takes %ld\n", (get_millis() - start_sending_packets));
	device_buffer[0] = MODE_SS;
	device_buffer[1] = SS_SYN;
	long int start_syn = get_millis();
	android_device_send_data_buffer(dev_id,device->packet_size,0);
	printf("Syn takes %ld\n", (get_millis() - start_syn));
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
	if(android_device_get_device_id(dev_id)->endianess == SAME){
		request = data_int[0];
		first_arg = data_int[1];
		second_arg = data_int[2];
		third_arg = data_int[3];
	}else{
		request = flipInt(data_int[0]);
		first_arg = flipInt(data_int[1]);
		second_arg = flipInt(data_int[2]);
		third_arg = flipInt(data_int[3]);
	}
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
			
			device->extra_data[0] = malloc(sizeof(ScreenGrabParams));
			ScreenGrabParams* params = device->extra_data[0];
			params->total_packets    = total_frames;
			params->data_per_packet  = frame_size;
			params->leftover_bytes   = leftover_bytes;

			while(1) sendScreenCapture(dev_id,params);
			break;

		case REQUEST_SS_START: device->transfer_status = TRANSFER_STATUS_SCREEN_SHARE; break;
		case REQEUST_SS_STOP:  device->transfer_status = TRANSFER_STATUS_WAITING; break;
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