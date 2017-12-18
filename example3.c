#include "xbee.h"


//The following example reads an ADC sample from a remote xbee: 
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
  
  //Pin we wish to read	
  unsigned char pin[] = {'D', '3'};	
  //Command name in ASCII characters:
  short int value = digitalReadXbee(fd, addr, pin);
  printf("Pin value: %i\n", value);
  
  return 0;
}
