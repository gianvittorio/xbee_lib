#include "xbee.h"


int open_port(void)
{
  int fd; /* File descriptor for the port */
  fd = open("/dev/ttyUSB0", O_RDWR | O_NDELAY | O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd == -1)
    {
      /*
       * Could not open the port.
       */
 
      perror("open_port: Unable to open /dev/ttyUSB0 - ");
    }
  else fcntl(fd, F_SETFL, fcntl(fd, F_GETFL ) & ~O_NONBLOCK);
    //fcntl(fd, F_SETFL, FNDELAY);
 
  //printf ( "In Open port fd = %i\n", fd);
 
  return (fd);
}


int setAttr(int fd)
{
    //Read the configureation of the port 
    struct termios options;
    tcgetattr( fd, &options );
 
    //Set Baud Rate 
    cfsetispeed( &options, B9600 );
    cfsetospeed( &options, B9600 );
  
    //Setting other Port Stuff 
    options.c_cflag &= ~PARENB; /*Make 8n1 */
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE; /* Mask the character size bits */
    options.c_cflag |= CS8;    /* Select 8 data bits */
    options.c_cflag &= ~CRTSCTS;  /* No flow control */
    options.c_cc[VMIN] = 1; /*READ doesn't block */
    options.c_cc[VTIME] = 1; /* 0.1 seconds read timout */
    options.c_cflag |= CREAD | CLOCAL; /* Turn on READ & ignore crtl lines */
 
    //Make raw
    cfmakeraw(&options);
  
    //Flush port, then applies attributes
    tcflush(fd, TCIFLUSH);

    return tcsetattr( fd, TCSANOW, &options );
}


void serialWrite(int fd, unsigned char value)
{
  int m = write(fd, &value, 1);
  
  if (m < 0)
    fputs("write() of bytes failed!\n", stderr);
  else
    printf ("%x written successfully (n = %i)\n", value, m);
}

 
void serialRead(int fd, unsigned char *value)
{
    *value = '\0';
    int n = 0; 
    n = read( fd, value, 1 );
    if (n < 0) {
    printf ( "Error = %s\n", strerror( errno ) );
    }
    else if (n == 0) {
      printf ( "Read Nothing...\n");
    }
    else {
      printf("Response: %x\n", *value);
    }
}

unsigned char calcChecksum(unsigned char *str, int n)
{
	long sum = 0;
	int i;
	for (i = 3; i < n; ++i)
		sum += str[i];
	
	unsigned char checksum = 0xFF - (sum & 0xFF);
	return checksum;
}


int getResponse(int fd, unsigned char *str)
{
	int i = 0;
   	for (;;)
   	{
       	serialRead(fd, &str[i]);
	    if (calcChecksum(str, i) == str[i])	
			break;
		
		i++;
   	}
	
	return (i + 1);
}


int digitalWriteXbee(int fd, unsigned char *addr, unsigned char *pin, unsigned char value)
{
   unsigned char frame[MAXBYTES];
	
   //Start byte:
   frame[0] = 0x7E;
   serialWrite(fd, frame[0]);
   //Length:
   frame[1] = 0x00;
   serialWrite(fd, frame[1]);
   frame[2] = 0x10;	
   serialWrite(fd, frame[2]);
   //Frame type:
   frame[3] = 0x17;
   serialWrite(fd, frame[3]);
   
   //Frame ID:
   frame[4] = 0x01;
   serialWrite(fd, frame[4]);
   
   //ID recipient:
   int i, n = MAXCHARS;
   for (i = 0; i < n; i++)
   {
	   frame[i + 5] = addr[i];
       serialWrite(fd, frame[i + 5]); 
   }
   
   //16 bit of recipient:
   frame[13] = 0xFF;
   serialWrite(fd, frame[13]);
   frame[14] = 0xFE;
   serialWrite(fd, frame[14]);
	
   //Set changes immediatly:
   frame[15] = 0x02;
   serialWrite(fd, frame[15]);
	
   //Command name in ASCII characters:
   for (i = 0; i < PINCOD; i++)
   {
       frame[i + 16] = pin[i];
       serialWrite(fd, frame[i + 16]);
   }
   
   //Command parameter:
   frame[18] = value;	
   serialWrite(fd, frame[18]);
	
   //Checksum:
   unsigned char checksum = calcChecksum(frame, 19);
   frame[19] = checksum;
   serialWrite(fd, frame[19]); 
	
   usleep(100000);  //give the XBee a chance to respond

   //Get AT response:
   unsigned char str[MAXBYTES];
   getResponse(fd, str); // Returns number of bytes received
 
   int r;	
   if (str[17] != 0x00) 
	   r = -1;
   else 
	   r = 0;
   
   return r;
}

short int digitalReadXbee(int fd, unsigned char *addr, unsigned char *pin)
{
   unsigned char frame[MAXBYTES];
	
   //Start byte:
   frame[0] = 0x7E;
   serialWrite(fd, frame[0]);
   //Length:
   frame[1] = 0x00;
   serialWrite(fd, frame[1]);
   frame[2] = 0x0F;	
   serialWrite(fd, frame[2]);
   //Frame type:
   frame[3] = 0x17;	
   serialWrite(fd, frame[3]);
   //Frame ID:
   frame[4] = 0x01;	
   serialWrite(fd, frame[4]);
   
   //ID recipient:
   int i, n = MAXCHARS;
   for (i = 0; i < n; i++)
   {
	   frame[i + 5] = addr[i];
	   serialWrite(fd, frame[i + 5]);
   }
   
   //16 bit of recipient:
   frame[13] = 0xFF;	
   serialWrite(fd, frame[13]);
   frame[14] = 0xFE;	
   serialWrite(fd, frame[14]);
   
   //Remote command options do not matter to IS (Queried Sample):
   frame[15] = 0x00;
   serialWrite(fd, frame[15]);
   
   //AT Command name IS in hex characters:
   frame[16] = 0x49;
   serialWrite(fd, frame[16]);
   frame[17] = 0x53;	 
   serialWrite(fd, frame[17]);
   
   //Checksum:
   unsigned char checksum = calcChecksum(frame, 18);
   frame[18] = checksum;
   serialWrite(fd, frame[18]); //send checksum
   
   usleep(100000);                  //give the XBee a chance to respond    

   
   //Get AT response:
   unsigned char str[MAXBYTES];
   getResponse(fd, str); // Returns number of bytes received
   unsigned char maskLow = 0x01,  maskHigh = 0x01, statusHigh = str[19], statusLow = str[20],
   valueHigh = str[21], valueLow = str[22], value; //Bytes 21 and 22 correspond to digital samples
   //printf("%x %x %x %x\n", statusHigh, statusLow, valueHigh, valueLow);	
   
   if ((str[17] != 0x00) && (pin[0] != 'D')) //The 17th byte being 0x00 stands for OK 
      return -1;
   else
   {
       //Check if pin is set as digital input on pin mask, then check if pin value is high ow low
	   switch(pin[1]) {
        case '0' :
          if ((statusLow & maskLow) == 0x01) 
			  value = ((valueLow && maskLow) == 0x01);
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }
		  break;
        case '1' :
		  if ((statusLow & (maskLow << 1)) == 0x02)	   
          	  value = ((valueLow & (maskLow << 1)) == 0x02);
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }
          break;
        case '2' :
		  if ((statusLow & (maskLow << 2)) == 0x04)	   
          	  value = ((valueLow & (maskLow << 2)) == 0x04);
          else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }  
		  break;
        case '3' :
		  if ((statusLow & (maskLow << 3)) == 0x08)	    
		  {
			  value = ((valueLow & (maskLow << 3)) == 0x08);
		  }
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n");
		  }
          break;
        case '4' :
          if ((statusLow & (maskLow << 4)) == 0x10)
			  value = ((valueLow & (maskLow << 4)) == 0x10);
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }
          break;
        case '5' :
          if ((statusLow & (maskLow << 5)) == 0x20)
			  value = ((valueLow & (maskLow << 5)) == 0x20);
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }
          break;
        case '6' :
		  if ((statusLow & (maskLow << 6)) == 0x40)
          	  value = ((valueLow & (maskLow << 6)) == 0x40);
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }
          break;
        case '7' :
		  if ((statusLow & (maskLow << 7)) == 0x80)
          	  value = ((valueLow & (maskLow << 7)) == 0x80);
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }
          break;
        case '8' :
          if ((statusHigh & maskHigh) == 0x01)
			  value = ((valueHigh & maskHigh) == 0x01);
		  else
		  {
			  value = -1;
			  printf("Error: pin is not set as digital input.\n"); 
		  }
          break;
        default :
          value = -1;
		  printf("Error: pin does not exist.\n");	   
       }
   }
   
   return value;
}



short int analogReadXbee(int fd, unsigned char *addr, unsigned char *pin)
{
   unsigned char frame[MAXBYTES];
	
   //Start byte:
   frame[0] = 0x7E;
   serialWrite(fd, frame[0]);
   //Length:
   frame[1] = 0x00;
   serialWrite(fd, frame[1]);
   frame[2] = 0x0F;
   serialWrite(fd, frame[2]);
   //Frame type:
   frame[3] = 0x17;
   serialWrite(fd, frame[3]);
   //Frame ID:
   frame[4] = 0x01;
   serialWrite(fd, frame[4]);
   
   //ID recipient:
   int i, n = MAXCHARS;
   for (i = 0; i < n; i++)
   {
	   frame[i + 5] =  addr[i];
	   serialWrite(fd, frame[i + 5]);
   }
   
   //16 bit of recipient:
   frame[13] = 0xFF;
   serialWrite(fd, frame[13]);
   frame[14] = 0xFE;
   serialWrite(fd, frame[14]);
   
   //Remote command options do not matter to IS (Queried Sample):
   frame[15] = 0x00;
   serialWrite(fd, frame[15]);
   
   //AT Command name IS in hex characters:
   frame[16] = 0x49;
   serialWrite(fd, frame[16]);
   frame[17] = 0x53;
   serialWrite(fd, frame[17]);
   
   //Checksum:
   unsigned char checksum = calcChecksum(frame, 18);
   frame[18] = checksum;
   serialWrite(fd, frame[18]); //send checksum
   
   usleep(100000);                  //give the XBee a chance to respond    

   
   //Get AT response:
   unsigned char str[MAXBYTES];
   getResponse(fd, str); // Returns number of bytes received
   int nSamples = ((int)str[2] - 20) / 2;
   //printf("Number of samples: %d\n", nSamples);	
   unsigned char mask = 0x02, status = str[19]; //Byte 19 corresponds to digital pin mask
   unsigned short pinMask[] = {0, 0, 0, 0, 0, 0, 0}; //MSB to LSB
   short sample;
   if ((str[17] != 0x00) && (pin[0] != 'A')) 
   {
	   return -1;
   }
   else
   {
        //Check if pin is set as analog input on pin mask
	   switch(pin[1]) {
        case '0' :
          if ((status & mask) == 0x02)
		  {
		  	pinMask[6] = 1;  
		  }
		  else
		  {
			  printf("Error: pin is not set as analog input.\n");
			  return -1;
		  }
		  break;
        case '1' :
		  if ((status & (mask << 1)) == 0x04)
		  {
			  pinMask[5] = 1;
		  }
		  else
		  {
			  printf("Error: pin is not set as analog input.\n");
			  return -1;
		  }
          break;
        case '2' :
		  if ((status & (mask << 2)) == 0x08)
		  {
          	  pinMask[4] = 1;
		  }
          else
		  {
			  printf("Error: pin is not set as analog input.\n");
			  return -1; 
		  }  
		  break;
        case '3' :
		  if ((status & (mask << 3)) == 0x10)
		  {
		  	  pinMask[3] = 1;
		  }
		  else
		  {
			  printf("Error: pin is not set as analog input.\n");
			  return -1;
		  }
          break;
        case '4' :
          if ((status & (mask << 4)) == 0x20)
		  {
			  pinMask[2] = 1;
		  }
		  else
		  {
			  printf("Error: pin is not set as analog input.\n");
			  return -1; 
		  }
          break;
        case '5' :
          if ((status & (mask << 5)) == 0x40)
		  {
			  pinMask[1] = 1;
		  }
		  else
		  {
			  printf("Error: pin is not set as analog input.\n");
			  return -1; 
		  }
          break;
        case '6' :
		  if ((status & (mask << 6)) == 0x80)
		  {
			  pinMask[0] = 1;
		  }
		  else
		  {
			  printf("Error: pin is not set as analog input.\n");
			  return -1; 
		  }
          break;
        default :
			{
          		printf("Error: pin does not exist.\n");
		   		return -1;
	   		}
       }
	   
	   int j = 0;
	   for (i = 0; i < 7; ++i)
	   {
			if (pinMask[i])
			{
				if (j > (nSamples - 1))
					break;
					
				sample = (short)(str[23 + j] << 8) + (short)str[24 + j];
				++j;
			}
	   }
	   
	   return sample;
   }
}   

int analogWriteXbee(int fd, unsigned char *addr, unsigned char *pin, unsigned char *value)
{
   //Send first frame in order to set M0 as PWM output
   unsigned char frame[MAXBYTES];
	
   //Start byte:
   frame[0] = 0x7E;
   serialWrite(fd, frame[0]);
   //Length:
   frame[1] = 0x00;
   serialWrite(fd, frame[1]);
   frame[2] = 0x10;	
   serialWrite(fd, frame[2]);
   //Frame type:
   frame[3] = 0x17;
   serialWrite(fd, frame[3]);
   
   //Frame ID:
   frame[4] = 0x05;
   serialWrite(fd, frame[4]);
   
   //ID recipient:
   int i, n = MAXCHARS;
   for (i = 0; i < n; i++)
   {
	   frame[i + 5] = addr[i];
       serialWrite(fd, frame[i + 5]); 
   }
   
   //16 bit of recipient:
   frame[13] = 0xFF;
   serialWrite(fd, frame[13]);
   frame[14] = 0xFE;
   serialWrite(fd, frame[14]);
	
   //Set changes immediatly:
   frame[15] = 0x02;
   serialWrite(fd, frame[15]);
	
   //Command parameter:
   for (i = 0; i < PINCOD; i++)
   {
       frame[i + 16] = pin[i];
       serialWrite(fd, frame[i + 16]);
   }	
   
   //Command parameter:
   frame[18] = 0x02;
   serialWrite(fd, frame[18]);
   	
	
   //Checksum:
   unsigned char checksum = calcChecksum(frame, 19);
   frame[19] = checksum;
   serialWrite(fd, frame[19]); 
	
   usleep(100000);  //give the XBee a chance to respond

   //Get AT response:
   unsigned char str[MAXBYTES];
   getResponse(fd, str); // Returns number of bytes received
 
   int r;	
   if (str[17] != 0x00) 
	   r = -1;
   else 
	   r = 0;
	
   usleep(100000);  //give the XBee a chance to respond	
	
   
   //Send second frame in order to set the dutycicle as requested. PWM period is 64 microsseconds.
	
   //Start byte:
   frame[0] = 0x7E;
   serialWrite(fd, frame[0]);
   //Length:
   frame[1] = 0x00;
   serialWrite(fd, frame[1]);
   frame[2] = 0x11;	
   serialWrite(fd, frame[2]);
   //Frame type:
   frame[3] = 0x17;
   serialWrite(fd, frame[3]);
   
   //Frame ID:
   frame[4] = 0x05;
   serialWrite(fd, frame[4]);
   
   //ID recipient:
   n = MAXCHARS;
   for (i = 0; i < n; i++)
   {
	   frame[i + 5] = addr[i];
       serialWrite(fd, frame[i + 5]); 
   }
   
   //16 bit of recipient:
   frame[13] = 0xFF;
   serialWrite(fd, frame[13]);
   frame[14] = 0xFE;
   serialWrite(fd, frame[14]);
	
   //Set changes immediatly:
   frame[15] = 0x02;
   serialWrite(fd, frame[15]);
	
   //Command name in ASCII characters:
   frame[16] = 0x4D;
   serialWrite(fd, frame[16]);
   frame[17] = 0x30;
   serialWrite(fd, frame[17]);	
   
   //Command parameter:
   frame[18] = value[0];	
   serialWrite(fd, frame[18]);
   frame[19] = value[1];	
   serialWrite(fd, frame[19]);	
	
   //Checksum:
   checksum = calcChecksum(frame, 20);
   frame[20] = checksum;
   serialWrite(fd, frame[20]); 
	
   usleep(100000);  //give the XBee a chance to respond

   //Get AT response:
   getResponse(fd, str); // Returns number of bytes received
 
   if (str[17] != 0x00) 
	   r = -1;
   else 
	   r = 0;	
   
   return r;
}


void convertSerial2ByteArray(char *serial, unsigned char *byteArray)
{
	int i;
	int n = strlen(serial);
	for (i = 0; i < n; ++i)
		byteArray[i] = (unsigned char)serial[i];
}


int lcdSend(int fd, unsigned char *addr, char *string)
{
	//Send first frame in order to set M0 as PWM output
   unsigned char frame[MAXBYTES];
	
   //Start byte:
   frame[0] = 0x7E;
   serialWrite(fd, frame[0]);
   //Length:
   frame[1] = 0x00;
   serialWrite(fd, frame[1]);
   unsigned char msgLen = (unsigned char)strlen(string);
   if (msgLen > 16)
   {
	printf("Error: Sending more than 16 characters now allowed\n");
	return -1;
   }	
   frame[2] = 11 + msgLen;	
   serialWrite(fd, frame[2]);
   //Frame type:
   frame[3] = 0x00;
   serialWrite(fd, frame[3]);
   
   //Frame ID:
   frame[4] = 0x01;
   serialWrite(fd, frame[4]);
   
   //ID recipient:
   int i, n = MAXCHARS;
   for (i = 0; i < n; i++)
   {
	   frame[i + 5] = addr[i];
       serialWrite(fd, frame[i + 5]); 
   }
   
   //Options:
   frame[13] = 0x00;
   serialWrite(fd, frame[13]);
   
   //RF data:
   unsigned char byteArray[16];
   convertSerial2ByteArray(string, byteArray);
   for (i = 0; i < strlen(string); i++)
   {
       frame[i + 14] = byteArray[i];
       serialWrite(fd, frame[i + 14]);
   }	
   
   //Checksum:
   unsigned char checksum = calcChecksum(frame, 14 + strlen(string));
   frame[14 + strlen(string) - 1] = checksum;
   serialWrite(fd, frame[14 + strlen(string) - 1]); 
	
   usleep(100000);  //give the XBee a chance to respond

   //Get AT response:
   unsigned char str[MAXBYTES];
   getResponse(fd, str); // Returns number of bytes received
 
   int r;	
   if (str[5] != 0x00) 
	   r = -1;
   else 
	   r = 0;

   return r;
} 
