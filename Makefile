# Copyright © 2014 Bart Massey

CC = gcc
CDEBUG = -O4
CFLAGS = -std=c99 -Wall $(CDEBUG)

duvis: duvis.o
	$(CC) $(CFLAGS) -o duvis duvis.o

clean:
	-rm -f duvis duvis.o core core.*
