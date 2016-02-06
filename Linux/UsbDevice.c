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

#include "UsbDevice.h"

static const int g_android_PID_VID_len = 1;
static int g_android_PID_array[1] = {20923};
static int g_android_VID_array[1] = {4046};

static int g_accessory_mode_VID[1] = {0x18d1};
static int g_accessory_mode_PID[1] = {0x2D01};

static void error(int code);
static int detatch_kernel_driver_error(int result);
static int claim_interface_error(int error);


const char* manufacturer = "Oleg T";
const char* modelName = "AndroidToKeyboard";
const char* protocol = "HID";
const char* version = "v.10";
const char* uri = "www.dankmaymays.com";
const char* serialnumber = "0";

UsbDevice findAndroidDevice(libusb_context* cntx);
int setToAccessoryMode(UsbDevice* device);
UsbDevice connectToAccessory(libusb_context* cntx);
void doHandshake(UsbDevice* dev);

/*
	Function does all the work
	1. Searches through all usb devices for a device whose PID and VID are in the VID/PID arrays
	2. Opens the found Device.
	3. Sends the contorl messages to the device to put it into accessory mode
	4. Frees the old device  
	5. Looks for a device whose VID and PID match the ones specified by google for an android phone in accessory mode
	6. Opens the device and takes contorl of the Interface
	7. Does a handshake to determine endianess and packet size
	8. Device is now ready for I/O
*/
int connectToAndroidDevice(libusb_context* cntx, UsbDevice* device){
	UsbDevice android_device = findAndroidDevice(cntx);
	if(android_device.is_valid == 0){
		printf("Failed Finding Device\n");
		return 1;
	}
	libusb_open(android_device.device, &(android_device.device_handle) );
	setToAccessoryMode(&android_device);
	freeDevice(&android_device);
	listDevices(cntx);

	*device = connectToAccessory(cntx);
	if(!device->is_valid){
		printf("Failed to connect to accesssory\n");
		return 1;
	}
	doHandshake(device);
	return 0;
};

/*
	Assumes phone writes using its own endianess
*/

static void* deviceReadThread(UsbDeviceReadListener* args){
	UsbDevice* device = args->dev;
	//void (*listener)(u_char* data, int length) = args->listener;
	char* read_buffer = malloc(sizeof(char)*device->packet_size);
	int result = 0;
	int read_bytes;
	printf("Opening Device Read Thread\n");
	while(device->is_valid){
		memset(read_buffer,0,device->packet_size);
		result = libusb_bulk_transfer(device->device_handle,IN,(unsigned char*)read_buffer,device->packet_size,&read_bytes,0);
		switch(result){
		    case 0: 
		    	printf("Successfully read %d bytes \n", read_bytes); break; 
    		case LIBUSB_ERROR_TIMEOUT: 
    			printf("ERROR: read timed out \n"); break; 
    		case LIBUSB_ERROR_PIPE: 
    			printf("ERROR: endpoint halted \n"); break;
    		case LIBUSB_ERROR_OVERFLOW: 
    			printf("ERROR: overflow \n"); break;
    		case LIBUSB_ERROR_NO_DEVICE: 
    			printf("ERROR: decice disconnected \n"); break; 
		}
		if(!result){
			(*args->listener)(read_buffer,read_bytes);
		}
	}
	free(read_buffer);
	freeDevice(read_buffer);
	printf("Closing Device Read Thread\n");
}
/*
	Does the handshake with the device that figures out the endianess and packet size to user for the sesion
	The device sends 32 bytes
	First 4 are control bytes to be interpreted as as int to determine if the endianess is different
	Second 4 bytes represet the size of the packet to be used from now on
	We then reply with our own controll message so the device can know our endianess
*/
void doHandshake(UsbDevice* dev){
	char buffer[32];
	int read_bytes;
	memset(buffer,0,32);
	int error = libusb_bulk_transfer(dev->device_handle,IN,(unsigned char*)buffer,32,&read_bytes,5000);
	printf("Handshake read %d bytes\n", read_bytes);
	if(error){
		printf("Failed Handshake\n");
		return;
	}
	int* a_ints = (int*)buffer;

	if(a_ints[0] == 1){ 
		dev->endianess = SAME;
		dev->packet_size = a_ints[1];
		printf("Endianess is same, packet size is %d\n",dev->packet_size);
	}else{
		dev->endianess = OPPOSITE;
		int converted_size;
		char* size_as_char = (char*)(&converted_size);
		size_as_char[0] = buffer[7];
		size_as_char[1] = buffer[6];
		size_as_char[2] = buffer[5];
		size_as_char[3] = buffer[4];
		dev->packet_size = converted_size;
		printf("Endianess is opposite, packet size %d\n",converted_size);
	}
	a_ints[0] = 1;
	
	error = sendData(dev,buffer,32);
	int retry_count = 0;
	while(error && retry_count < 3){
		printf("Retrying\n");
		retry_count++;
		error = sendData(dev,buffer,32);
		if(retry_count == 3 && error){
			return;
		}
	}
	printf("Handshake success\n");
	printDevice(dev);
}


/*
	Synchronously sends data to the android device
	Sends data in the endianess of the recieving device
*/
int  sendData(UsbDevice* device, void* data, int length){
	/*
	int libusb_bulk_transfer 	( 	
		struct libusb_device_handle *  	dev_handle,
		unsigned char  	endpoint,
		unsigned char *  	data,
		int  	length,
		int *  	transferred,
		unsigned int  	timeout 
	) 
	*/
	int result;
	int transferred_bytes;
	result = libusb_bulk_transfer(device->device_handle,OUT,(unsigned char*)data,length,&transferred_bytes,1500);
	switch(result){
	    case 0: 
	    	printf("Successfully transfered %d bytes \n", transferred_bytes); break; 
    	case LIBUSB_ERROR_TIMEOUT: 
    		printf("ERROR: transfer timed out \n"); break; 
    	case LIBUSB_ERROR_PIPE: 
    		printf("ERROR: endpoint halted \n"); break;
    	case LIBUSB_ERROR_OVERFLOW: 
    		printf("ERROR: overflow \n"); break;
    	case LIBUSB_ERROR_NO_DEVICE: 
    		printf("ERROR: decice disconnected \n"); break; 
	}	
	return result;
}


/*
	Looks through Every attatched device, and finds the one that has the a VID and PID that is presend in the VID and PID arrays
	References the device is finds
*/
UsbDevice findAndroidDevice(libusb_context* cntx){ //finds and references the device
	UsbDevice ret_device;
	ret_device.is_valid = 0;
	ret_device.is_open = 0;
	ret_device.is_accessory = 0;
	ret_device.interface = 0;

	libusb_device **devs;
	ssize_t dev_count = libusb_get_device_list(cntx, &devs);
	printf("There are %d attatched devices\n",dev_count);
	struct libusb_device_descriptor device_descrptn;
	int count = 0;
	while(count < dev_count){
		libusb_get_device_descriptor(devs[count],&device_descrptn);
		uint16_t vendor_id  = device_descrptn.idVendor;
		uint16_t product_id = device_descrptn.idProduct;
		printf("	VENDOR_ID: %d, PRODUCT_ID: %d \n", vendor_id,product_id);
		int vid_pid= 0;
		while(vid_pid < g_android_PID_VID_len){
			if(g_android_VID_array[vid_pid] == vendor_id && g_android_PID_array[vid_pid] == product_id){
				ret_device.is_valid = 1;
				ret_device.device = devs[count];
				libusb_ref_device(ret_device.device); 	
				libusb_free_device_list(devs, 0);
				printf("Found Device \n");
				return ret_device;
			}
			++vid_pid;
		}
		++count;
	}
	libusb_free_device_list(devs, 0);
	return ret_device;

};

void listDevices(libusb_context* cntx){
	libusb_device **devs;
	ssize_t dev_count = libusb_get_device_list(cntx, &devs);
	printf("There are %d attatched devices\n",dev_count);
	struct libusb_device_descriptor device_descrptn;
	int count = 0;
	while(count < dev_count){
		libusb_get_device_descriptor(devs[count],&device_descrptn);
		uint16_t vendor_id  = device_descrptn.idVendor;
		uint16_t product_id = device_descrptn.idProduct;
		printf("	VENDOR_ID: %d, PRODUCT_ID: %d \n", vendor_id,product_id);
		count++;
	}
}

/*
	Requires that the device is already opened
	Sets the device into accessory mode
*/
int setToAccessoryMode(UsbDevice* device){
	printf("Setting device to accessory mode\n");
	int response;

	response = libusb_control_transfer(device->device_handle,0x40,52,0,0,(unsigned char*)manufacturer,strlen(manufacturer),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(device->device_handle,0x40,52,0,1,(unsigned char*)modelName,strlen(modelName),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(device->device_handle,0x40,52,0,2,(unsigned char*)protocol,strlen(protocol),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(device->device_handle,0x40,52,0,3,(unsigned char*)version,strlen(version),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(device->device_handle,0x40,52,0,4,(unsigned char*)uri,strlen(uri),0);
	if(response < 0){error(response);return 1;}

	response = libusb_control_transfer(device->device_handle,0x40,52,0,5,(unsigned char*)serialnumber,strlen(serialnumber),0);
	if(response < 0){error(response);return 1;}	

	response = libusb_control_transfer(device->device_handle,0x40,53,0,0,NULL,0,0);
	if(response < 0){error(response);return 1;}

	usleep(1500000); //wait for device to enter accesssory mode
	return 0;
};


UsbDevice connectToAccessory(libusb_context* cntx){
	UsbDevice ret_device;
	ret_device.is_valid = 0;
	ret_device.is_open = 0;
	ret_device.is_accessory = 0;
	ret_device.interface = 0;

	ret_device.device_handle = libusb_open_device_with_vid_pid(cntx,g_accessory_mode_VID[0],g_accessory_mode_PID[0]);
	if(ret_device.device_handle == NULL){
		printf("Failed to find accesssory\n");
		return ret_device;
	}
	ret_device.is_valid = 1;
	ret_device.is_accessory = 1;
	ret_device.is_open = 1;
	ret_device.device = libusb_get_device (ret_device.device_handle);
	int res;
	res = libusb_detach_kernel_driver(ret_device.device_handle, ret_device.interface);
	if(detatch_kernel_driver_error(res)){
		ret_device.is_valid = 0;
		return ret_device;
	}
	res = libusb_claim_interface(ret_device.device_handle, ret_device.interface);	
	if(claim_interface_error(res)){
		ret_device.is_valid = 0;
		return ret_device;
	}
	return ret_device;

}

void printDevice(UsbDevice* dev){
	printf("Device: \n");
	printf("	Vendor id: %d\n",dev->vendor_id);
	printf("	Product id: %d\n", dev->product_id);
	printf("	Is Valid: %d\n",dev->is_valid);
	printf("	Is Open: %d\n", dev->is_open);
	printf("	State: %d\n",dev->state);
	printf("	Packet size: %d\n",dev->packet_size);
	printf("	Endianess: %d\n",dev->endianess);
};

void freeDevice(UsbDevice* device){
	printf("Freeing device\n");
	if(!device->is_valid){
		return;
	}
	device->is_valid = 0;
	if(device->is_open){
		printf("	Device was open, Closing\n");
		libusb_close(device->device_handle);
		libusb_release_interface(device->device_handle,device->interface);	
	}
	libusb_unref_device(device->device);
};



pthread_t startListening(UsbDeviceReadListener* listener){
	pthread_t thread;
	pthread_create(&thread, NULL, deviceReadThread, listener);
	return thread;
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