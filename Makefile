# Copyright © 2014 Bart Massey
# [This program is licensed under the "MIT License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.


CC = gcc
CDEBUG = -O4
CFLAGS = -std=c99 -Wall $(CDEBUG)

duvis: duvis.o
	$(CC) $(CFLAGS) -o duvis duvis.o

clean:
	-rm -f duvis duvis.o
