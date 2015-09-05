CFLAGS=-std=c99 -pedantic -Wall -O3

all: pfkpk test

pfkpk: pfkpk.c kpk.c kpk.h
	$(CC) $(CFLAGS) -o $@ pfkpk.c kpk.c

test:
	./pfkpk

# vi: noexpandtab
