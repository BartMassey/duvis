# Copyright © 2014 Bart Massey

CC = gcc
CFLAGS = -std=c99 -Wall -O4

duvis: duvis.o
	$(CC) $(CFLAGS) -o duvis duvis.o
