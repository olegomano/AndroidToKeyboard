#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

int  fb_init();
void fb_print();
int fb_get_size_byte();
void fb_cpframe(void* out);

#endif