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

#define TYPE_CONF 1
#define TYPE_DATA 2
#define TYPE_CLS 3

#define PACKET_HANDHSAKE 1
#define PACKET_DATA 2
#define PACKET_CLOSE 3    

static void error(int code);
static int detatch_kernel_driver_error(int result);
static int claim_interface_error(int error);


const char* manufacturer = "Oleg T";
const char* modelName = "AndroidToKeyboard";
const char* protocol = "HID";
const char* version = "v.10";
const char* uri = "www.dankmaymays.com";
const char* serialnumber = "0";


int accessory_hotplug(struct libusb_context *ctx, struct libusb_device *dev,libusb_hotplug_event event, void *user_data);
int device_hotplug(struct libusb_context *ctx, struct libusb_device *dev,libusb_hotplug_event event, void *user_data);
AndroidDevice* android_device_match_port(uint8_t* path);
void android_device_swap_endianess(char* in, char* out, int length);

libusb_context* cntx;
AndroidDevice** devices;
int max_device = 100;
int alloced_device_count = 0;

int android_device_create_context(){
	libusb_init(&cntx);
	libusb_set_debug(cntx, 3); //set verbosity level to 3, as suggested in the documentation
	devices = malloc(sizeof(AndroidDevice*) * max_device);
	memset(devices,0,sizeof(AndroidDevice*) * max_device);
	return 0;
}

/*
	Register for hotplug events for a device with the specified version id / product id
	Returns a integer representing the device

*/
int android_device_reg(int vid, int pid){
	AndroidDevice* n_dev = malloc(sizeof(AndroidDevice));
	
	n_dev->vendor_id = vid;
	n_dev->product_id = pid;
	n_dev->conncetion_status = CONNECTION_STATUS_DISCONNECTED;
	n_dev->transfer_status = TRASNFER_STATUS_HANDSHAKE;
	n_dev->buffer = NULL;
	
	int m_id = alloced_device_count;
	n_dev->dev_id = m_id;

	devices[alloced_device_count++] = n_dev;
	

	libusb_hotplug_callback_handle android_handle;
	libusb_hotplug_callback_handle accessory_handle;
	
	int rc; 
	rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, vid, pid,
												LIBUSB_HOTPLUG_MATCH_ANY, device_hotplug, NULL,
												&android_handle);

	if(rc != LIBUSB_SUCCESS){
		printf("Failed Singing up for Hotplug events\n");
		return -1;
	}
	
	//this one should be put in the create_context
	rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, 0x18d1, 0x2D01,
												LIBUSB_HOTPLUG_MATCH_ANY, accessory_hotplug, NULL,
												&accessory_handle);
	
	if(rc != LIBUSB_SUCCESS){
		printf("Failed Singing up for Hotplug events\n");
		return -1;
	}

	return m_id;
}

void android_device_read_thread(int dev_id){
	AndroidDevice* device = devices[dev_id];
	char* read_packet = malloc(READ_PACKET_SIZE);
	int read_bytes;
	char packet_type;
	int  read_err;
	while(device->conncetion_status == CONNECTION_STATUS_CONNECTED){
		memset(read_packet,0,READ_PACKET_SIZE);
		read_err = libusb_bulk_transfer(device->device_handle,IN,read_packet,READ_PACKET_SIZE,&read_bytes,0);
		if(read_err!=0){ //there is an error reading
			switch(read_err){
				case LIBUSB_ERROR_TIMEOUT: 
    				printf("ERROR: read timed out \n"); break; 
    			case LIBUSB_ERROR_PIPE: 
    				printf("ERROR: endpoint halted \n"); break;
    			case LIBUSB_ERROR_OVERFLOW: 
    				printf("ERROR: overflow \n"); break;
    			case LIBUSB_ERROR_NO_DEVICE: 
    				printf("ERROR: decice disconnected \n"); break; 
			}
			continue;
		}
		int* read_packet_int = (int*)read_packet;
		printf("%08x %08x %08x %08x\n", read_packet_int[0],read_packet_int[1],read_packet_int[2],read_packet_int[3]);
		packet_type = read_packet[0];
		printf("Packet Type: %d\n", packet_type);
		if(device->transfer_status == TRASNFER_STATUS_HANDSHAKE){
			if(packet_type != PACKET_HANDHSAKE){
				printf("Error: protocol mismatch, Expecting handshake request\n");
				continue;
			}else{
				//do handshake
				printf("Handshake in Progress\n");
				int* packet_payload = read_packet + sizeof(int);
				int  controll_int = packet_payload[0];
				int  packet_size;
				printf("Controll Int %08x\n", controll_int);
				printf("Packet Size: %d\n",packet_payload[1]);
				if(controll_int == 1){
					device->endianess = SAME;
					packet_size = packet_payload[1];
				}else{
					device->endianess = OPPOSITE;
					android_device_swap_endianess( (char*) (&packet_payload[1]) , (char*)(&packet_size), sizeof(int) );
				}
				if(device->buffer != NULL){
					free(device->buffer);
				}
				device->packet_size = packet_size;
				device->buffer = malloc(packet_size);
				device->buffer_size = packet_size;
				memset(device->buffer,0,packet_size);
				char handshake_reply[1024];
				int* handshake_reply_int = (int*)handshake_reply;
				handshake_reply_int[0] = 1;
				handshake_reply_int[1] = READ_PACKET_SIZE;
				if(!android_device_send_data(dev_id,handshake_reply,1024)){
					printf("Handshake Success\n");
					android_device_print_device(dev_id);
					device->transfer_status = TRANSFER_STATUS_WAITING;
				};

			}
		}else{
			switch(packet_type){
				case PACKET_DATA: device->callback.onDataRead(dev_id,read_packet + sizeof(char) );  break;
				case PACKET_CLOSE: device->transfer_status = TRASNFER_STATUS_HANDSHAKE; break;
			}
		}

	}
}


/*
	Gets the hotplug events for a device in accessoy mode.
	All Anroid devices in accessory mode have the same VID/PID. 
	Beucause of this I destinguish between accessory mode devices using their port address. 
	The assumption is that when a device is connected in Android Mode, and then reconnects in Accessory Mode it will
	be on the same port adress. 

*/
int accessory_hotplug(struct libusb_context *ctx, struct libusb_device *dev,libusb_hotplug_event event, void *user_data){
	printf("An Accessory Has Arived\n");
	uint8_t device_port_path[7];
	libusb_get_port_numbers(dev,device_port_path,7);
	AndroidDevice* matched_dev = android_device_match_port(device_port_path);
	if(matched_dev != NULL){
		if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED){
			printf("An Accessory Has Arived\n");

			int res;
			matched_dev->device = dev;

			res = libusb_open(matched_dev->device,&matched_dev->device_handle);
			
			error(res);
					
			res = libusb_detach_kernel_driver(matched_dev->device_handle,0);
			
			if(detatch_kernel_driver_error(res)){
			//	return 0;
			}

			res = libusb_claim_interface(matched_dev->device_handle, 0);	
			if(claim_interface_error(res)){
			//	return 0;
			}
			
			matched_dev->conncetion_status = CONNECTION_STATUS_CONNECTED;
			matched_dev->transfer_status = TRASNFER_STATUS_HANDSHAKE;
			android_device_print_device(matched_dev->dev_id);
			pthread_t thread;
			pthread_create(&thread, NULL, android_device_read_thread,matched_dev->dev_id);
			//android_device_handshake(matched_dev->dev_id);
		}else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT){
			printf("An Accessory Has Left\n");
			matched_dev->conncetion_status = CONNECTION_STATUS_DISCONNECTED;
			android_device_print_device(matched_dev->dev_id);	
		}
	}

	return 0;
}

/*
	When a Device Is connected wtih a specified VID / PID in android_device_reg this callback is invoked
	Using the VIP/PID I find my AndroiDevice which is registered to the same VID/PID. 
*/
int device_hotplug(struct libusb_context *ctx, struct libusb_device *dev,libusb_hotplug_event event, void *user_data){
	printf("Device Hotplug Event\n");
	struct libusb_device_descriptor device_descrptn;
	libusb_get_device_descriptor(dev,&device_descrptn);
	AndroidDevice* m_dev = android_device_get_device(device_descrptn.idVendor,device_descrptn.idProduct);
	if(m_dev != NULL){
		if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED){
			printf("An Android Device has Arrived\n");
			libusb_get_port_numbers(dev,m_dev->port_numbers,7);
			
			android_device_print_device(m_dev->dev_id);

			libusb_device_handle* handle;
			libusb_open(dev,&handle);


			printf("Setting device to accessory mode\n");
			int response;
			response = libusb_control_transfer(handle,0x40,52,0,0,(unsigned char*)manufacturer,strlen(manufacturer),0);
			if(response < 0){error(response);return 0;}
		
			response = libusb_control_transfer(handle,0x40,52,0,1,(unsigned char*)modelName,strlen(modelName),0);
			if(response < 0){error(response);return 0;}

			response = libusb_control_transfer(handle,0x40,52,0,2,(unsigned char*)protocol,strlen(protocol),0);
			if(response < 0){error(response);return 0;}

			response = libusb_control_transfer(handle,0x40,52,0,3,(unsigned char*)version,strlen(version),0);
			if(response < 0){error(response);return 0;}

			response = libusb_control_transfer(handle,0x40,52,0,4,(unsigned char*)uri,strlen(uri),0);
			if(response < 0){error(response);return 0;}

			response = libusb_control_transfer(handle,0x40,52,0,5,(unsigned char*)serialnumber,strlen(serialnumber),0);
			if(response < 0){error(response);return 0;}	

			response = libusb_control_transfer(handle,0x40,53,0,0,NULL,0,0);
			if(response < 0){error(response);return 0;}
		}
	}
	return 0;

}



int android_device_destroy_context(){

}

int android_device_poll_events(){
	libusb_handle_events(cntx);
}


int android_device_send_data(int dev_id, unsigned char* data, int length){
	int transferred_bytes;
	int result = libusb_bulk_transfer(devices[dev_id]->device_handle,OUT,data,length,&transferred_bytes,7500);
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

int android_device_send_data_buffer(int dev_id, int length, int timeout){
	AndroidDevice* dev = devices[dev_id];
	int transferred_bytes;
	if(length > dev->packet_size){
	//	printf("Error sending, length larger than packet size\n");
	}
	int result = libusb_bulk_transfer(dev->device_handle,OUT,dev->buffer,length,&transferred_bytes,timeout);
	switch(result){
	    case 0: break; 
	    	//printf("Successfully transfered %d bytes \n", transferred_bytes); break; 
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

int android_device_set_callbacks(int dev_id, AndroidDeviceCallbacks callback){
	devices[dev_id]->callback = callback;
}

void android_device_swap_endianess(char* in, char* out, int length){
	int i;
	for(i = 0; i < length; i++){
		out[i] = in[length - 1 - i];
	}
}

AndroidDevice* android_device_match_port(uint8_t* path){
	int i;
	for(i = 0; i < alloced_device_count; i++){
		printf("Checking Dev %d\n",i );
		AndroidDevice* dev = devices[i];
		int c;
		int wrong = 0;
		for(c = 0; c < 1; c++){
			printf("Given Path: %d Device Path: %d\n",path[c],dev->port_numbers[c]);
			if(path[c] != dev->port_numbers[c]){
				++wrong;
			}
		}
		if(wrong == 0){
			return dev;
		}
	}
	return NULL;
}


AndroidDevice* android_device_get_device(int vid, int pid){
	int i;
	for(i = 0; i < alloced_device_count; i++){
		AndroidDevice* device = devices[i];
		if(device->vendor_id == vid && device->product_id == pid){
			printf("Found Device with VID %d, PID %d\n",vid,pid);
			return device;
		}
	}
	return NULL;
}

AndroidDevice* android_device_get_device_id(int dev_id){
	return devices[dev_id];
}

void android_device_print_device(int dev_id){
	AndroidDevice* dev = devices[dev_id];
	printf("Device %d\n", dev_id);
	printf("  VID: %d\n", dev->vendor_id);
	printf("  PID: %d\n", dev->product_id);
	printf("  Packet Size: %d\n", dev->packet_size);
	printf("  Endianess:   %d\n", dev->endianess);
	printf("  Connection:  %d\n", dev->conncetion_status);
	printf("  Transfer:    %d\n", dev->transfer_status);
	printf("  Port Path: ");
	int i;
	for(i = 0; i < 7; i++){
		printf("%d ", dev->port_numbers[i]);
	}
	printf("\n");
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
		default: 
			printf("ERROR: OTHER ERROR\n");
		return 1;//failure
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