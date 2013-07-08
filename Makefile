CC=gcc
CLIBS=-lhiredis
CFLAGS=-Wall -W -pedantic

mirtill: mirtill.o params.o levenshtein.o
	$(CC) $(CLIBS) *.o -o mirtill
mirtill.o: mirtill.c
	$(CC) $(CFLAGS) mirtill.c -c
levenshtein.o: levenshtein.c levenshtein.h
	$(CC) $(CFLAGS) levenshtein.c -c
params.o: params.c params.h
	$(CC) $(CFLAGS) params.c -c

clean:
	rm -f *.o mirtill
