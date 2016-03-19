#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "FrameBuffer.h"

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int   fb_fdesc;
int   fb_w;
int   fb_h;
int   fb_bpp;  //bits per pixel
int   fb_size;
unsigned long fb_mem_start;
void* fb;


int fb_init(){
	fb_fdesc = open("/dev/fb0", O_RDWR);
	if(fb_fdesc == -1){
		perror("Failed to open /dev/fb0 \n");
		return 0;
	}
	if (ioctl(fb_fdesc, FBIOGET_FSCREENINFO, &finfo) == -1) {
		perror("Failed to get fixed screen info\n");
		return 0;
	}
	if(ioctl(fb_fdesc,FBIOGET_VSCREENINFO,&vinfo) == -1){
		perror("Failed to get variable screen info\n");
		return 0;
	}
	fb_w = vinfo.xres;
	fb_h = vinfo.yres;
	fb_bpp = vinfo.bits_per_pixel;
	fb_size = finfo.smem_len;
	fb_mem_start = finfo.smem_start;
	fb = mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fdesc, 0);
	if(fb == NULL){
		perror("Failed to mmap framebuffer");
		return 0;
	}
	printf("Mapped FrameBuffer\n");
	return 1;
}

int fb_get_size_byte(){
	return fb_size;
}

void fb_print(){
	printf("FrameBuffer w,h, bpp, size, mstart: %d %d %d %d %d\n", fb_w,fb_h,fb_bpp,fb_size,fb_mem_start);
}

void fb_cpframe(void* out){
	int pixel = ((int*)(fb))[1];
	printf("%04x\n",pixel);
	memcpy(fb,out,fb_size);
}


int fb_destroy(){

}

int fb_get_w(){
	return fb_w;
}

int fb_get_h(){
	return fb_h;
}
