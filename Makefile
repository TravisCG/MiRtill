CC=gcc
CLIBS=-lhiredis
CFLAGS=-Wall -W -pedantic

mirtill: mirtill.o
	$(CC) $(CLIBS) *.o -o mirtill
mirtill.o: mirtill.c
	$(CC) $(CFLAGS) mirtill.c -c

clean:
	rm -f *.o mirtill
