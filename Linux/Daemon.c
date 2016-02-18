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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <usb.h>
#include <libusb-1.0/libusb.h>
#include "UsbDevice.h"
#include "Daemon.h"


FILE* log_file;
FILE* getLogFile(){
	return log_file;
}

int daemonize(DaemonMain f){
	pid_t pid;
	pid_t sid;

	pid = fork();
	//fork creates a child process, the PID of the child process is 0, and the parent is a positive numeber
	//if the pid is negative then the fork failed
	if(pid < 0){
		exit(EXIT_FAILURE);
	}
	if(pid > 0){
		exit(EXIT_SUCCESS);	
	}
	umask(0);
	log_file = fopen ("daemonlog.txt", "wb");
	sid = setsid();
    if (sid < 0) {
    	DAEMON_FILE_PRINT("Daemon Error: failed to get sid");
    	fclose(log_file);
        exit(EXIT_FAILURE);
    }  
    if ((chdir("/")) < 0) {
 		DAEMON_FILE_PRINT("Daemon Error: failed to chdir");
 		fclose(log_file);
        exit(EXIT_FAILURE);
    }     	
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    f(pid);
    
    fclose(log_file);
};