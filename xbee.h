#ifndef XBEE_H_
#define XBEE_H_

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#include <string.h>
#include <bsd/string.h>

#define MAXCHARS 8
#define MAXBYTES 100
#define PINCOD 2 // 2 bytes are necessary to codify a pin. Ex: 'D' , '3'

int open_port(void); //Open serial port 1. Returns the file descriptor on success or -1 on error.
int setAttr(int fd); //Set serial port configurations. Returns -1 on error.
int digitalWriteXbee(int fd, unsigned char *addr, unsigned char *pin, unsigned char value); //Enter remote xbee serial number and pin, then writes desired value to pin. Returns -1 on error.
short int digitalReadXbee(int fd, unsigned char *addr, unsigned char *pin); //Returns short int indicating 16 bit mask indicating digital pins status, -1 for error. 
short int analogReadXbee(int fd, unsigned char *addr, unsigned char *pin); //Reads ADC value from remote xbee. Returns short int with a value from 0 to 1023 for success, -1 for error.
int analogWriteXbee(int fd, unsigned char *addr, unsigned char *pin, unsigned char *value); //Writes 0 to 1023 integer to PWM0 pin. Returns -1 for error.
int lcdSend(int fd, unsigned char *addr, char *string); //Sends string to LCD 16x2 display. Returns number of bytes received as reply

#endif /* XBEE_H_ */
