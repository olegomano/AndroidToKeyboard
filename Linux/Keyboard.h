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