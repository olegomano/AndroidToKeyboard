
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>

#define _VENDOR_ID_ 4046
#define _PRODUCT_ID_ 20923
#define _INTERFACE_ 0


struct android_usb_device{
	libusb_device* device;
	libusb_device_handle* device_handle;
};

libusb_device* getAndroidDevice(libusb_device** device_list, int device_count);
void printdev(libusb_device *dev); //prototype of the function
int startListening(android_usb_device* dev);
void cleanup(android_usb_device* dev, libusb_device **dev_list);

bool g_is_running = true;
pthread_t g_poll_thread_info;	
libusb_context* g_libusb_context = NULL;


int main() {
	android_usb_device android_usb;
	android_usb.device = NULL;
	android_usb.device_handle = NULL;

	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	int r; //for return values
	ssize_t cnt; //holding number of devices in list
	r = libusb_init(&g_libusb_context); //initialize a library session
	if(r < 0) {
		printf("Init Error "); //there was an error
			return 1;
	}
	libusb_set_debug(g_libusb_context, 3); //set verbosity level to 3, as suggested in the documentation
	cnt = libusb_get_device_list(g_libusb_context, &devs); //get the list of devices
	if(cnt < 0) {
		printf("Get Device Error"); //there was an error
	}
	
	printf("There are %d devices \n", cnt); //print total number of usb devices
	android_usb.device = getAndroidDevice(devs,cnt);
	if(android_usb.device == NULL){
		cleanup(&android_usb,devs);
		return 1;
	}
	printdev(android_usb.device);
	int err = startListening(&android_usb);
	if(err){
		cleanup(&android_usb,devs);
		return 1;	
	}

	std::string user_input;
	while(true){
		std::cin >> user_input;
		if(user_input == "exit"){
			break;
		}
	}
	cleanup(&android_usb,devs);
	return 0;
}

void cleanup(android_usb_device* dev, libusb_device **dev_list){
	g_is_running = false;
	libusb_release_interface(dev->device_handle,_INTERFACE_);
	libusb_close(dev->device_handle);
	libusb_free_device_list(dev_list, 0); //free the list, unref the devices in it
	libusb_exit(g_libusb_context); //close the session
}



libusb_device* getAndroidDevice(libusb_device** device_list, int device_count){
	int i=0;
	while(i < device_count){
		libusb_device_descriptor device_descrptn;
		libusb_get_device_descriptor(device_list[i],&device_descrptn);
		uint16_t vendor_id  = device_descrptn.idVendor;
		uint16_t product_id = device_descrptn.idProduct;
		printf("Looking at device %d, VendorID: %d, ProductID: %d\n",i,vendor_id,product_id);
		//printdev(device_list[i]);
		if(vendor_id == _VENDOR_ID_ && product_id == _PRODUCT_ID_){
			return device_list[i];
		}
		++i;
	}
	printf("Failed to find Android device\n");
	return NULL;
}

void onUSBThreadStarted(){
	printf("USB Thread started\n");
}

void onUSBThreadDied(){
	printf("USB Thread died\n");
}

void* usbEventThread(void* args){
	onUSBThreadStarted();
	while(g_is_running){
		printf("Handling usb Events %d\n",g_is_running);
		libusb_handle_events(g_libusb_context);
	}
	onUSBThreadDied();
}



int startListening(android_usb_device* android_device){
	printf("Got Android Device\n");
	if(  !libusb_open(android_device->device,&(android_device->device_handle))){ //succefully opened device, libusb_open returns 0 on success
		int result;
		libusb_set_configuration(android_device->device_handle,0);
		result = libusb_detach_kernel_driver(android_device->device_handle, _INTERFACE_);
		switch(result){
			case 0: 
				printf("DETACHED KERNEL DRIVER, attempting to claim interface\n");
				break;
			case LIBUSB_ERROR_NOT_FOUND: 
				printf("NO KERNEL DRIVER DETECTED, attempting to claim interface\n");
				break;
			case LIBUSB_ERROR_INVALID_PARAM: 
				printf("ERROR DETATCHIG KERNEL DRIVER: interace does not exist\n"); 
				return 1;
			case 
				LIBUSB_ERROR_NO_DEVICE: printf("ERROR DETATCHIG KERNEL DRIVER: the device has been disconnected\n"); 
				return 1;
			case LIBUSB_ERROR_NOT_SUPPORTED: 
				printf("ERROR DETATCHING KERNEL DRIVER: on platforms where the functionality is not available\n");
				return 1;
			default: 
				printf("ERROR DETATCHIG KERNEL DRIVER: unspecified error");
				return 1;
		}
		if(libusb_kernel_driver_active(android_device->device_handle,_INTERFACE_)){
			printf("Kernel Driver active on Interface: %d", _INTERFACE_);
		}
		result = libusb_claim_interface(android_device->device_handle, _INTERFACE_);
		switch(result){
			case 0: 
				printf("CLAIMED DEVICE INTERFACE: starting callback I/O thread\n");
				pthread_create (&g_poll_thread_info, NULL, usbEventThread, NULL);
				return 0;
			case LIBUSB_ERROR_NOT_FOUND: 
				printf("ERROR CLAIMING INTERFACE: could not find interface\n");
				return 1;
			case LIBUSB_ERROR_BUSY: 
				printf("ERROR CLAIMING INTERFACE: device is busy\n");
				return 1;
			case LIBUSB_ERROR_NO_DEVICE: 
				printf("ERROR CLAIMING INTERFACE: could not find device\n");
				return 1;
			default: return 1;//failure
		}
	}else{
		return 1;
		printf("ERROR: failed opening usb device\n");
	}
	return 0;


}

void printdev(libusb_device *dev) {
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		printf("failed to get device descriptor\n");
		return;
	}
	printf( "Number of possible configurations: %d\n",(int)desc.bNumConfigurations);
	printf( "Device Class: %d \n", (int)desc.bDeviceClass );
	printf( "VendorID: %d \n", desc.idVendor);
	printf("ProductID: %d \n",desc.idProduct);
	libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	printf("Interfaces: %d \n",(int)config->bNumInterfaces);
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;
	for(int i=0; i<(int)config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		printf("	Number of alternate settings: %d \n",inter->num_altsetting);
		for(int j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			printf("		Interface Number: %d \n",(int)interdesc->bInterfaceNumber);
			printf("		Number of endpoints: %d \n",(int)interdesc->bNumEndpoints);
			for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				printf("			Descriptor Type: %d\n",(int)epdesc->bDescriptorType );
				printf("			EP Address: %d\n",(int)epdesc->bEndpointAddress );
			}
		}
	}
	printf("\n\n\n");
	libusb_free_config_descriptor(config);
}