#include "X11Wrapper.h"

XImage* x11_shminit();

Display* display;
Window   root_window;
XWindowAttributes attr;
x11FBuffer framebuffer;

XImage* xshm_image;
XShmSegmentInfo sh_img_info;

int xWindow_w;
int xWindow_h;


int x11_init(){
	display = XOpenDisplay(NULL);
	root_window = XRootWindow(display,0);
	XGetWindowAttributes(display,root_window,&attr);
	xWindow_w = attr.width;
	xWindow_h = attr.height;
	framebuffer.w = xWindow_w;
	framebuffer.h = xWindow_h;
	//framebuffer.fb_data = malloc(sizeof(pixel) * xWindow_w * xWindow_h);
	xshm_image = x11_shminit();
	framebuffer.fb_data = malloc(xWindow_w * xWindow_h * 4);
};

x11FBuffer* x11_getframe(){
	if(!XShmGetImage(display,root_window,xshm_image,0,0,AllPlanes)){
		printf("Failed get shmgetImage\n");
		return;
	}
	int i;
	for(i = 0; i < xWindow_h * xWindow_w; i++){
		framebuffer.fb_data[i].a = ((pixel*)xshm_image->data)[i].b;
        framebuffer.fb_data[i].r = ((pixel*)xshm_image->data)[i].g;
		framebuffer.fb_data[i].g = ((pixel*)xshm_image->data)[i].r;
		framebuffer.fb_data[i].b = ((pixel*)xshm_image->data)[i].a;
	}

	/*
	int pixel;
	for(pixel = 0; pixel < (framebuffer.w * framebuffer.h) ; pixel++){
		printf("%02x %02x %02x %02x\n", 
								   framebuffer.fb_data[pixel].r, 
								   framebuffer.fb_data[pixel].g,
								   framebuffer.fb_data[pixel].b,
								   framebuffer.fb_data[pixel].a);
	}
	*/
	return &framebuffer;
}

int x11_get_fb_w(){
	return xWindow_w;
}

int x11_get_fb_h(){
	return xWindow_h;
}

x11FBuffer* x11_getframe_slow(){
	XImage* image;
	XColor  pixel;
	image = XGetImage (display,root_window,0,0,attr.width,attr.height,AllPlanes,ZPixmap);
	/*
	int x;
	int y;
	int width = attr.width;
	int height = attr.height;
	for(x = 0; x < width; x++){
		for(y = 0; y < height; y++){
			//pixel.pixel = XGetPixel(image,x,y);
			framebuffer.fb_data[y*height + x].r = pixel.red;
			framebuffer.fb_data[y*height + x].g = pixel.green;
			framebuffer.fb_data[y*height + x].b = pixel.blue;
		}
	}
	*/
	return &framebuffer;
};

XImage* x11_shminit(){
	XImage* image;

	image = XShmCreateImage(display, DefaultVisual(display,0),32,ZPixmap,NULL,&sh_img_info, xWindow_w, xWindow_h);
	if(image == NULL){
		printf("Failed alloc image\n");
		return image;
	}
	u_long mem_size = image->bytes_per_line*image->height;
	framebuffer.depth = mem_size / (xWindow_w * xWindow_h);
	printf("Shmem size: %d, %d\n",mem_size,framebuffer.depth);
	sh_img_info.shmid = shmget(IPC_PRIVATE, image->bytes_per_line*image->height,IPC_CREAT | 0777 );
    if(sh_img_info.shmid < 0) {
    	printf("Shmid failed\n");
    	return 1;
	}
	/* attach, and check for errrors */
    sh_img_info.shmaddr = image->data = (char *)shmat(sh_img_info.shmid, 0, 0);
    if(sh_img_info.shmaddr == (char *) -1) {
    	printf("Failed tp attach memmory\n");
    	return image;
    }
    /* set as read/write, and attach to the display */
    sh_img_info.readOnly = False;
    XShmAttach(display, &sh_img_info);
	return image;
}

int x11_destroy(){

};

void x11_print(){
	int screen_count = XScreenCount(display);
	printf("Screen count: %d\n",screen_count );
	printf("Window w,h: %d,%d\n", attr.width,attr.height);
}
