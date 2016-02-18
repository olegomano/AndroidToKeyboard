#ifndef _DAEMON_H_
#define _DAEMON_H_
#define DAEMON_FILE_PRINT(...) fprintf(getLogFile(), __VA_ARGS__); 

FILE* getLogFile();
typedef void (*DaemonMain)(int pid);
int daemonize( DaemonMain main );

#endif