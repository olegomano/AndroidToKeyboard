#include "Keyboard.h"

static volatile int uinput_fd;


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
	
	unsigned char enabled_key;
	int converted_keycode;
	for(enabled_key = 0; enabled_key < 255; enabled_key++){
		converted_keycode = getKeyCode(enabled_key);
		if(converted_keycode >= 0){
			res = ioctl(uinput_fd, UI_SET_KEYBIT, converted_keycode);
			printf("%d Enabling key %d\n", enabled_key, converted_keycode);
		}
	}

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

void shiftUp(){
	usleep(1500);
	printf("Shift up.\n");
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.type = EV_KEY;
	ev.code = KEY_LEFTSHIFT;
	ev.value = 0;
	int res = write(uinput_fd, &ev, sizeof(ev));
	usleep(1500);
	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	res = write(uinput_fd, &ev, sizeof(ev));
}

void shiftDown(){
	printf("Shift Down\n");
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.type = EV_KEY;
	ev.code = KEY_LEFTSHIFT;
	ev.value = 1;
	int res = write(uinput_fd, &ev, sizeof(ev));
	usleep(1500);
	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	res = write(uinput_fd, &ev, sizeof(ev));
	usleep(1500);
}


void sendKeyDown(char key){
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	int key_code = getKeyCode(key);
	if(key_code == 0) return;
	if(key_code < 0){
		shiftDown();
		key_code = -key_code;
	}
	ev.type = EV_KEY;
	ev.code = key_code;
	ev.value = 1;
	int res = write(uinput_fd, &ev, sizeof(ev));
	printf("Sending Key Down event for %d\n",ev.code );
	usleep(1500);
	
	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	res = write(uinput_fd, &ev, sizeof(ev));
	if(res < 0 )printf("Error sending Key Down\n");
}

void sendKeyUp(char key){
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	int key_code = getKeyCode(key);
	ev.type = EV_KEY;
	ev.code = key_code;
	if(ev.code == 0) return;
	if(key_code < 0){
		ev.code = -key_code;
	}
	ev.value = 0 ;
	int res = write(uinput_fd, &ev, sizeof(ev));
	printf("Sending Key Up event for %d\n",ev.code );
	usleep(1500);
	
	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	res = write(uinput_fd, &ev, sizeof(ev));
	if(key_code < 0){
		shiftUp();
	}
	if(res < 0 )printf("Error sending Key UP\n");
};

int getKeyCode(unsigned char key){
	int uinput_keycode;
	switch(key){
		case 'a': uinput_keycode = KEY_A; break;
		case 'b': uinput_keycode = KEY_B; break;
		case 'c': uinput_keycode = KEY_C; break;
		case 'd': uinput_keycode = KEY_D; break;
		case 'e': uinput_keycode = KEY_E; break;
		case 'f': uinput_keycode = KEY_F; break;
		case 'g': uinput_keycode = KEY_G; break;
		case 'h': uinput_keycode = KEY_H; break;
		case 'i': uinput_keycode = KEY_I; break;
		case 'j': uinput_keycode = KEY_J; break;
		case 'k': uinput_keycode = KEY_K; break;
		case 'l': uinput_keycode = KEY_L; break;
		case 'm': uinput_keycode = KEY_M; break;
		case 'n': uinput_keycode = KEY_N; break;
		case 'o': uinput_keycode = KEY_O; break;
		case 'p': uinput_keycode = KEY_P; break;
		case 'q': uinput_keycode = KEY_Q; break;
		case 'r': uinput_keycode = KEY_R; break;
		case 's': uinput_keycode = KEY_S; break;
		case 't': uinput_keycode = KEY_T; break;
		case 'u': uinput_keycode = KEY_U; break;
		case 'v': uinput_keycode = KEY_V; break;
		case 'w': uinput_keycode = KEY_W; break;
		case 'x': uinput_keycode = KEY_X; break;
		case 'y': uinput_keycode = KEY_Y; break;
		case 'z': uinput_keycode = KEY_Z; break;
		case '1': uinput_keycode = KEY_1; break;	 
		case '2': uinput_keycode = KEY_2; break;	 
		case '3': uinput_keycode = KEY_3; break;	 
		case '4': uinput_keycode = KEY_4; break;	 
		case '5': uinput_keycode = KEY_5; break;	 
		case '6': uinput_keycode = KEY_6; break;	 
		case '7': uinput_keycode = KEY_7; break;	 
		case '8': uinput_keycode = KEY_8; break;	 
		case '9': uinput_keycode = KEY_9; break;	 
		case '0': uinput_keycode = KEY_0; break;

		case 'A': uinput_keycode = -KEY_A; break;
		case 'B': uinput_keycode = -KEY_B; break;
		case 'C': uinput_keycode = -KEY_C; break;
		case 'D': uinput_keycode = -KEY_D; break;
		case 'E': uinput_keycode = -KEY_E; break;
		case 'F': uinput_keycode = -KEY_F; break;
		case 'G': uinput_keycode = -KEY_G; break;
		case 'H': uinput_keycode = -KEY_H; break;
		case 'I': uinput_keycode = -KEY_I; break;
		case 'J': uinput_keycode = -KEY_J; break;
		case 'K': uinput_keycode = -KEY_K; break;
		case 'L': uinput_keycode = -KEY_L; break;
		case 'M': uinput_keycode = -KEY_M; break;
		case 'N': uinput_keycode = -KEY_N; break;
		case 'O': uinput_keycode = -KEY_O; break;
		case 'P': uinput_keycode = -KEY_P; break;
		case 'Q': uinput_keycode = -KEY_Q; break;
		case 'R': uinput_keycode = -KEY_R; break;
		case 'S': uinput_keycode = -KEY_S; break;
		case 'T': uinput_keycode = -KEY_T; break;
		case 'U': uinput_keycode = -KEY_U; break;
		case 'V': uinput_keycode = -KEY_V; break;
		case 'W': uinput_keycode = -KEY_W; break;
		case 'X': uinput_keycode = -KEY_X; break;
		case 'Y': uinput_keycode = -KEY_Y; break;
		case 'Z': uinput_keycode = -KEY_Z; break;
		case '!': uinput_keycode = -KEY_1; break;	 
		case '@': uinput_keycode = -KEY_2; break;	 
		case '#': uinput_keycode = -KEY_3; break;	 
		case '$': uinput_keycode = -KEY_4; break;	 
		case '%': uinput_keycode = -KEY_5; break;	 
		case '^': uinput_keycode = -KEY_6; break;	 
		case '&': uinput_keycode = -KEY_7; break;	 
		case '*': uinput_keycode = -KEY_8; break;	 
		case '(': uinput_keycode = -KEY_9; break;	 
		case ')': uinput_keycode = -KEY_0; break;

		case ' ': uinput_keycode = KEY_SPACE; break;
		case 14:  uinput_keycode = KEY_LEFTSHIFT; break;
		case 0:  uinput_keycode = KEY_BACKSPACE; break;
		case 10:   uinput_keycode = KEY_ENTER; break;

		case '=': uinput_keycode = KEY_KPPLUS; break;
		case '-': uinput_keycode = KEY_KPMINUS; break;
		case '\\': uinput_keycode = KEY_BACKSLASH; break;
		case '[': uinput_keycode = KEY_LEFTBRACE; break;
		case ']': uinput_keycode = KEY_RIGHTBRACE; break;
		case '\'':uinput_keycode = KEY_APOSTROPHE; break;
		case ';': uinput_keycode = KEY_SEMICOLON; break;
		case '/': uinput_keycode = KEY_SLASH; break;
		case '.': uinput_keycode = KEY_DOT; break;
		case ',': uinput_keycode = KEY_COMMA; break;

		case '+': uinput_keycode = -KEY_KPPLUS; break;
		case '_': uinput_keycode = -KEY_KPMINUS; break;
		case '|': uinput_keycode = -KEY_BACKSLASH; break;
		case '{': uinput_keycode = -KEY_LEFTBRACE; break;
		case '}': uinput_keycode = -KEY_RIGHTBRACE; break;
		case '"':uinput_keycode = -KEY_APOSTROPHE; break;
		case ':': uinput_keycode = -KEY_SEMICOLON; break;
		case '?': uinput_keycode = -KEY_SLASH; break;
		case '>': uinput_keycode = -KEY_DOT; break;
		case '<': uinput_keycode = -KEY_COMMA; break;

		default: uinput_keycode = 0; break;

	}
	return uinput_keycode;
}

void sendMouse(int dx, int dy){

};
