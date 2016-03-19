#ifndef __X11WRAPPER_H__
#define __X11WRAPPER_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h> 

typedef struct{ //blue green red format
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
} pixel;

typedef struct {
	unsigned char b;
	unsigned char g;
	unsigned char r;
} pixelAndroid;

typedef struct{
	int depth;
	int w;
	int h;
	pixel* fb_data;
} x11FBuffer;

int  		 x11_init();
x11FBuffer*  x11_getframe();
int  		 x11_destroy();
void 		 x11_print();
int          x11_get_fb_w();
int 		 x11_get_fb_h();

#endif