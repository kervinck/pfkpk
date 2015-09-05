CFLAGS=-std=c99 -pedantic -Wall -O3

all: pfkpk test

pfkpk: pfkpk.c kpk.c kpk.h
	$(CC) $(CFLAGS) -o $@ pfkpk.o kpk.o

test:
	./pfkpk

clean:
	rm -f *.o

# vi: noexpandtab
