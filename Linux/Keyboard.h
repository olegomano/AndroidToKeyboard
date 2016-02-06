#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <string.h> 
#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>   


int openUinput();
void closeUinput();

void keyPress(char key);
void sendKeyDown(char key);
void sendKeyUp(char key);
void sendMouse(int dx, int dy);
int getKeyCode(unsigned char key);

#endif