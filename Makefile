CC = gcc
CFLAGS  = -g -Wall
#INCLUDES = -I/...
LIBS = -lbsd

example: example1.o example2.o  example3.o xbee.o
	$(CC) $(CFLAGS) example1.o xbee.o -o example1 $(LIBS)
	$(CC) $(CFLAGS) example2.o xbee.o -o example2 $(LIBS)
	$(CC) $(CFLAGS) example3.o xbee.o -o example3 $(LIBS)
	
example1.o:
	$(CC) $(CFLAGS) -c example1.c $(LIBS)

example2.o:
	$(CC) $(CFLAGS) -c example2.c $(LIBS)

example3.o:
	$(CC) $(CFLAGS) -c example3.c $(LIBS)

xbee.o:	xbee.c	xbee.h
	$(CC) $(CFLAGS) -c xbee.c $(LIBS)

clean:
	rm *.o *. *~