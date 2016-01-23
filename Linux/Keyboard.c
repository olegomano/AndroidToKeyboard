#include "Keyboard.h"
int uinput_fd;

int openUinput(){
	uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(uinput_fd < 0){
		return 1;
	}
	int res;
	res = ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
	if(res < 0) return 1;
	res =ioctl(uinput_fd, UI_SET_EVBIT, EV_SYN);	
	if(res < 0) return 1;
	res = ioctl(uinput_fd, UI_SET_KEYBIT, KEY_D);
	if(res < 0) return 1;
	
	struct uinput_user_dev uidev;	
	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, 80, "uinput-sample");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1234;
	uidev.id.product = 0xfedc;
	uidev.id.version = 1;
	res = write(uinput_fd, &uidev, sizeof(uidev));
	if(res < 0) return 1;
	res = ioctl(uinput_fd, UI_DEV_CREATE);
	if(res < 0) return 1;
	return 0;
};

void closeUinput(){
	ioctl(uinput_fd, UI_DEV_DESTROY);
}

void sendKeyDown(char key){
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.type = EV_KEY;
	ev.code = KEY_D;
	ev.value = 1;
	int res = write(uinput_fd, &ev, sizeof(ev));
	if(res < 0 ) printf("Failed to send Key %s\n", key);
	memset(&ev, 0, sizeof(ev));
	ev.type = EV_KEY;
	ev.code = KEY_D;
	ev.value = 0;
	res = write(uinput_fd, &ev, sizeof(ev));
	if(res < 0 ) printf("Failed to send Key %s\n", key);
};
void sendKeyUp(char key){};
void sendMouse(int dx, int dy){};
