# Copyright © 2014 Bart Massey
# [This program is licensed under the "MIT License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.


NAME = duvis
SRCS = duvis.h duvis.c graphics.c
OBJS = duvis.o graphics.o
CC = gcc
CDEBUG = -O4 # -pg -fprofile-arcs -ftest-coverage
CFLAGS = -std=c99 -Wall -g $(CDEBUG) \
	 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`

duvis:	$(OBJS)	
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LIBS)

$(OBJS): duvis.h

clean:
	-rm -f $(OBJS) duvis 
