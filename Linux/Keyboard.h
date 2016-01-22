#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

void openUinput();

void sendKeyDown(char key);
void sendKeyUp(char key);
void sendMouse(int dx, int dy);

#endif