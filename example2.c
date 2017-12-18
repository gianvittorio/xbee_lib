#include "xbee.h"


//The following example sets a digital pin to HIGH, then LOW, in an infinite loop, on a remote xbee: 
int main()
{
  int fd;  // File descriptor
  
  fd = open_port();
  if ( setAttr(fd) != 0 )
    printf ("Error with tcsetattr = %s\n", strerror ( errno ) );
  //else
    //printf ( "%s\n", "tcsetattr succeed" );
 
  // Serial Number of the router/end point
  unsigned char addr[] = {0x00, 0x13, 0xA2, 0x00, 0x40, 0xE3, 0x10, 0x1A}; //Serial address
  
  //Command name in ASCII characters:
  unsigned char pin[] = {'D', '4'};
  //Write desired state (0x05 for on, 0x04 for off)
  char value;
  int r;
  for (;;)
  {
  	value = 0x5;  
  	r = digitalWriteXbee(fd, addr, pin, value);
  	if (r == -1)
    	fputs("write() of bytes failed!\n", stderr);
  
  	usleep(100000);                  //give the XBee a chance to respond
  
  	value = 0x4;  
  	r = digitalWriteXbee(fd, addr, pin, value);
  	if (r == -1)
    	fputs("write() of bytes failed!\n", stderr);
  }
  
  close(fd);
 
  return 0;
}

  




  
