# Copyright © 2014 Bart Massey
# [This program is licensed under the "MIT License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.


NAME = duvis
SRCS = duvis.c
CC = gcc
CDEBUG = -O4
CFLAGS = -std=c99 -Wall -g $(CDEBUG) 
GTKFLAGS = -export-dynamic `pkg-config --cflags --libs gtk+-3.0`

duvis:	$(SRCS)	
	$(CC) $(CFLAGS) $(SRCS) $(GTKFLAGS) -o $(NAME) 

clean:
	-rm -f duvis 
