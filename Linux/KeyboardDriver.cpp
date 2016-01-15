#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>

#define _ERROR_ -1000

struct usb_descriptor{
	uint16_t vendor_id;
	uint16_t product_id;
};

struct usb_dev{
	bool is_accesory;
	usb_descriptor descriptor;
	int interface = _ERROR_;
	libusb_device* device;
	libusb_device_handle* device_handle;

};

const static int g_device_count = 1;
static int g_accesory_mode_vid[1] = {0x18d1};
static int g_accesory_mode_pid[2] = {0x2D01,0x2D00};
static int g_device_vid[g_device_count] = {4046};
static int g_device_pid[g_device_count] = {20923};


static void error(int code);
static int detatch_kernel_driver_error(int result);
static int claim_interface_error(int error);

bool g_usb_lister_thread = true;

usb_dev find_device(usb_descriptor* desc, int pos_dev_count, libusb_context* cntx);
usb_dev* accesory_mode_devices();

void printUsbDescriptor(usb_descriptor* usb);
int openDevice(usb_dev* dev);
int freeDevice(usb_dev* dev);
int putIntoAccessoryMode(usb_dev* anroid_device); //register myself as an android accesory 
int createDeviceIDList(usb_descriptor** usb_dev_list);
int createAccessoryIDList(usb_descriptor** usb_acc_list);

int main(){
	printf("Hello World\n");
	libusb_context* cntx;
	libusb_init(&cntx);
	libusb_set_debug(cntx, 3); //set verbosity level to 3, as suggested in the documentation
	
	usb_descriptor* usb_device_list;
	usb_descriptor* usb_acc_list;

	createDeviceIDList(&usb_device_list);
	createAccessoryIDList(&usb_acc_list);
	printf("Looking for device: ");
	printUsbDescriptor(usb_device_list);
	printf("Possible acessory mode PID and VID: \n");
	for(int i = 0; i < 2; i++){
		printUsbDescriptor(&usb_acc_list[i]);
	}

	usb_dev android_device = find_device(usb_device_list,g_device_count,cntx);
	
	if(android_device.interface == _ERROR_){ //didnt find regular device, see if it's already in accessory mode
		printf("Failed to find Android device, looking for device in Acccessory mode\n");
		android_device = find_device(usb_acc_list,2,cntx);
		if(android_device.interface == _ERROR_){ 
			libusb_exit(cntx);
			printf("Failed to find device in acessory mode, closing \n");
			return 1;	
		}
		printf("Device already in acessory mode, attemptint to open\n");
	}else{ //found device in regular mode, attempt to put it into acessory mode
		printf("Attempting to put device into acessory mode\n");
		int error;
		error = putIntoAccessoryMode(&android_device);
		if(error){
			printf("Failed to put device into accessory mode, closing\n");
			freeDevice(&android_device);
			libusb_exit(cntx);
			return 1;	
		}
		printf("Put device into accessory mode, attempting to find accessory\n");
		android_device = find_device(usb_acc_list,2,cntx);
		if(android_device.interface == _ERROR_){
			libusb_exit(cntx);
			printf("Failed to find Device in accessory mode, closing \n");
			return 1;	
		}
	}
	printf("Successfully put device into acessory mode, attemptint to take control of interface\n");
	openDevice(&android_device);
	while(true){
		//start thread to process data from device		
	}
	freeDevice(&android_device);
	delete[](usb_device_list);
	delete[](usb_acc_list);
	return 0;
}

void* usbEventListener(){
	while(g_usb_lister_thread){

	}
}

int freeDevice(usb_dev* dev){
	printf("Freeing device\n");
	libusb_release_interface(dev->device_handle,dev->interface);
	libusb_close(dev->device_handle);
	return 0;
}

int openDevice(usb_dev* dev){
	int res;
	res = libusb_open(dev->device,&(dev->device_handle));
	if(res){
		printf("Failed to open device\n");
		return 1;
	}
	res = libusb_detach_kernel_driver(dev->device_handle, dev->interface);
	if(detatch_kernel_driver_error(res)){
		return 1;
	};
	res = libusb_claim_interface(dev->device_handle, dev->interface);
	if(claim_interface_error(res)){
		return 1;
	}
	printf("Successfully claimed usb device interface\n");
	return 0;	
}


int putIntoAccessoryMode(usb_dev* anroid_device){
	int res = libusb_open(anroid_device->device,&(anroid_device->device_handle));

	const char* manufacturer = "Lenovo";
	const char* modelName = "Thinkpad";
	const char* protocol = "Testing protocol";
	const char* version = "1.0";
	const char* uri = "www.dankmaymay.org";
	const char* serialnumber = "10101010";

	int response;
	unsigned char ioBuffer[2];
	int devVersion;
	
	response = libusb_control_transfer(
		anroid_device->device_handle, //handle
		0xC0, //bmRequestType
		51, //bRequest
		0, //wValue
		0, //wIndex
		ioBuffer, //data
		2, //wLength
        0 //timeout
	);

	if(response < 0){error(response);return-1;}
	devVersion = ioBuffer[1] << 8 | ioBuffer[0];
	fprintf(stdout,"Verion Code Device: %d\n", devVersion);
	
	usleep(1000);

	response = libusb_control_transfer(anroid_device->device_handle,0x40,52,0,0,(unsigned char*)manufacturer,strlen(manufacturer),0);
	if(response < 0){error(response);return 1;}
	
	response = libusb_control_transfer(anroid_device->device_handle,0x40,52,0,1,(unsigned char*)modelName,strlen(modelName),0);
	if(response < 0){error(response);return 1;}
	
	response = libusb_control_transfer(anroid_device->device_handle,0x40,52,0,2,(unsigned char*)protocol,strlen(protocol),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(anroid_device->device_handle,0x40,52,0,3,(unsigned char*)version,strlen(version),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(anroid_device->device_handle,0x40,52,0,4,(unsigned char*)uri,strlen(uri),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(anroid_device->device_handle,0x40,52,0,5,(unsigned char*)serialnumber,strlen(serialnumber),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(anroid_device->device_handle,0x40,53,0,0,NULL,0,0);
	if(response < 0){error(response);return 1;}

	freeDevice(anroid_device);

	usleep(1500000);

	return 0;

}

usb_dev find_device(usb_descriptor* desc,int desc_count,libusb_context* cntx){
	libusb_device **devs;
	ssize_t dev_count = libusb_get_device_list(cntx, &devs);
	printf("There are %d attatched devices\n",dev_count);
	libusb_device* device;
	libusb_device_descriptor device_descrptn;

	for(int count = 0; count < dev_count; count++){
		device = devs[count];
		libusb_get_device_descriptor(device,&device_descrptn);
		uint16_t vendor_id  = device_descrptn.idVendor;
		uint16_t product_id = device_descrptn.idProduct;
		printf("Device %d, VENDOR_ID: %d, PRODUCT_ID %d \n",count,vendor_id,product_id);
		for(int i = 0; i < desc_count;i++ ){
			if(vendor_id == desc[i].vendor_id && product_id == desc[i].product_id){
				usb_dev found_dev;
				found_dev.descriptor = *desc;
				found_dev.interface = 0;
				found_dev.device = device;
				printf("TARGET DEVICE FOUND\n");
				libusb_free_device_list(devs, 0); //free the list, unref the devices in it
				return found_dev;
			}
		}
		
	}
	usb_dev f;
	f.interface = _ERROR_;
	return f;
}


int createDeviceIDList(usb_descriptor** usb_dev_list){
	*usb_dev_list = new usb_descriptor[g_device_count];
	for(int i = 0; i < g_device_count; i++){
		usb_dev_list[0][i].vendor_id = g_device_vid[i];
		usb_dev_list[0][i].product_id = g_device_pid[i];
	}
	return g_device_count;
}


int createAccessoryIDList(usb_descriptor** usb_dev_list){
	*usb_dev_list = new usb_descriptor[2];
	for(int i = 0; i < 2; i++){
		for(int b = 0; b < 1; b++){
			usb_dev_list[0][i*1 + b].vendor_id = g_accesory_mode_vid[b];
			usb_dev_list[0][i*1 + b].product_id = g_accesory_mode_pid[i];	
		}
	}
	return 2;
}

void printUsbDescriptor(usb_descriptor* usb){
	printf("PRODUCT_ID: %d ",usb->product_id);
	printf("VENDOR_ID: %d\n",usb->vendor_id );
}


static int claim_interface_error(int result){
	switch(result){
		case 0: 
			printf("CLAIMED DEVICE INTERFACE: success\n");
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
}

static int detatch_kernel_driver_error(int result){
	switch(result){
			case 0: 
				printf("DETACHED KERNEL DRIVER, success\n");
				return 0;
			case LIBUSB_ERROR_NOT_FOUND: 
				printf("NO KERNEL DRIVER DETECTED, success\n");
				return 0;
			case LIBUSB_ERROR_INVALID_PARAM: 
				printf("ERROR DETATCHIG KERNEL DRIVER: interace does not exist\n"); 
				return 1;
			case 
				LIBUSB_ERROR_NO_DEVICE: 
				printf("ERROR DETATCHIG KERNEL DRIVER: the device has been disconnected\n"); 
				return 1;
			case LIBUSB_ERROR_NOT_SUPPORTED: 
				printf("ERROR DETATCHING KERNEL DRIVER: on platforms where the functionality is not available\n");
				return 1;
			default: 
				printf("ERROR DETATCHIG KERNEL DRIVER: unspecified error");
				return 1;
		}	
}


static void error(int code){
	fprintf(stdout,"\n");
	switch(code){
	case LIBUSB_ERROR_IO:
		fprintf(stdout,"Error: LIBUSB_ERROR_IO\nInput/output error.\n");
		break;
	case LIBUSB_ERROR_INVALID_PARAM:
		fprintf(stdout,"Error: LIBUSB_ERROR_INVALID_PARAM\nInvalid parameter.\n");
		break;
	case LIBUSB_ERROR_ACCESS:
		fprintf(stdout,"Error: LIBUSB_ERROR_ACCESS\nAccess denied (insufficient permissions).\n");
		break;
	case LIBUSB_ERROR_NO_DEVICE:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_DEVICE\nNo such device (it may have been disconnected).\n");
		break;
	case LIBUSB_ERROR_NOT_FOUND:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_FOUND\nEntity not found.\n");
		break;
	case LIBUSB_ERROR_BUSY:
		fprintf(stdout,"Error: LIBUSB_ERROR_BUSY\nResource busy.\n");
		break;
	case LIBUSB_ERROR_TIMEOUT:
		fprintf(stdout,"Error: LIBUSB_ERROR_TIMEOUT\nOperation timed out.\n");
		break;
	case LIBUSB_ERROR_OVERFLOW:
		fprintf(stdout,"Error: LIBUSB_ERROR_OVERFLOW\nOverflow.\n");
		break;
	case LIBUSB_ERROR_PIPE:
		fprintf(stdout,"Error: LIBUSB_ERROR_PIPE\nPipe error.\n");
		break;
	case LIBUSB_ERROR_INTERRUPTED:
		fprintf(stdout,"Error:LIBUSB_ERROR_INTERRUPTED\nSystem call interrupted (perhaps due to signal).\n");
		break;
	case LIBUSB_ERROR_NO_MEM:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_MEM\nInsufficient memory.\n");
		break;
	case LIBUSB_ERROR_NOT_SUPPORTED:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_SUPPORTED\nOperation not supported or unimplemented on this platform.\n");
		break;
	case LIBUSB_ERROR_OTHER:
		fprintf(stdout,"Error: LIBUSB_ERROR_OTHER\nOther error.\n");
		break;
	default:
		fprintf(stdout, "Error: unkown error\n");
	}
}

static void status(int code){
	fprintf(stdout,"\n");
	switch(code){
		case LIBUSB_TRANSFER_COMPLETED:
			fprintf(stdout,"Success: LIBUSB_TRANSFER_COMPLETED\nTransfer completed.\n");
			break;
		case LIBUSB_TRANSFER_ERROR:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_ERROR\nTransfer failed.\n");
			break;
		case LIBUSB_TRANSFER_TIMED_OUT:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_TIMED_OUT\nTransfer timed out.\n");
			break;
		case LIBUSB_TRANSFER_CANCELLED:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_CANCELLED\nTransfer was cancelled.\n");
			break;
		case LIBUSB_TRANSFER_STALL:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_STALL\nFor bulk/interrupt endpoints: halt condition detected (endpoint stalled).\nFor control endpoints: control request not supported.\n");
			break;
		case LIBUSB_TRANSFER_NO_DEVICE:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_NO_DEVICE\nDevice was disconnected.\n");
			break;
		case LIBUSB_TRANSFER_OVERFLOW:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_OVERFLOW\nDevice sent more data than requested.\n");
			break;
		default:
			fprintf(stdout,"Error: unknown error\nTry again(?)\n");
			break;
	}
}
