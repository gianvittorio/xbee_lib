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
  
  //Command name in ASCII characters:
  unsigned char pin[] = {'D', '2'};
  double sample;
  short int N = analogReadXbee(fd, addr, pin);
  if (N != -1)
  { 
      sample = (3.3/1023.0)*(double)N;
      printf("Sample value: %i\n", N);
      printf("Voltage: %lf V\n", sample);
  }
 
 
  return 0;
}
