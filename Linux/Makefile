
all: link

debug: Main.o UsbDevice.o Keyboard.o FrameBuffer.o X11Wrapper.o
	gcc Main.o UsbDevice.o Keyboard.o FrameBuffer.o X11Wrapper.o -g -o KeyboardDriver -lusb-1.0 -lpthread -lX11 -lXext -lm

link: Main.o UsbDevice.o Keyboard.o FrameBuffer.o X11Wrapper.o
	gcc Main.o UsbDevice.o Keyboard.o FrameBuffer.o X11Wrapper.o -o KeyboardDriver -lusb-1.0 -lpthread -lX11 -lXext -lm

Main.o: Main.c  
	gcc -c Main.c -o Main.o 

UsbDevice.o: UsbDevice.c 
	gcc -c UsbDevice.c -o UsbDevice.o 

Keyboard.o: UinputWrapper.c
	gcc -c UinputWrapper.c -o Keyboard.o 

FrameBuffer.o: FrameBuffer.c
	gcc -c FrameBuffer.c -o FrameBuffer.o

X11Wrapper.o: X11Wrapper.c
	gcc -c X11Wrapper.c -o X11Wrapper.o

clean:
	rm -f Main.o
	rm -f UsbDevice.o
	rm -f Keyboard.o
	rm -f FrameBuffer.o
	rm -f X11Wrapper.o

KeyboardDriver.o: KeyboardDriver.cpp
	g++ -std=c++11 KeyboardDriver.cpp -lusb-1.0 -lpthread
