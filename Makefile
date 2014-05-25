# Copyright © 2014 Bart Massey
# [This program is licensed under the "MIT License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.


CC = gcc
CDEBUG = -O4
CFLAGS = -std=c99 -Wall $(CDEBUG)
GTKFLAGS = `pkg-config --cflags --libs gtk+-3.0`

duvis: 
	$(CC) duvis.c $(GTKFLAGS) $(CFLAGS) -o duvis 

clean:
	-rm -f duvis duvis.o
