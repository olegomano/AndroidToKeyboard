
all: link

KeyDriver.o: KeyDriver.c  
	gcc -c KeyDriver.c -o KeyDriver.o 

UsbDevice.o: UsbDevice.c 
	gcc -c UsbDevice.c -o UsbDevice.o 

Keyboard.o: Keyboard.c
	gcc -c Keyboard.c -o Keyboard.o 

link: KeyDriver.o UsbDevice.o Keyboard.o
	gcc KeyDriver.o UsbDevice.o Keyboard.o -o KeyboardDriver -lusb-1.0 -lpthread

clean:
	rm -f KeyDriver.o
	rm -f UsbDevice.o
	rm -f Keyboard.o

KeyboardDriver.o: KeyboardDriver.cpp
	g++ -std=c++11 KeyboardDriver.cpp -lusb-1.0 -lpthread
