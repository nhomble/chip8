CC=gcc
CFLAGS=-O3 --pedantic -Wall -Werror
LDFLAGS=-lncurses
PROGS=chocolate_chip test

all: $(PROGS)

chip8.o: chip8.c chip8.h font.h
	$(CC) $(CFLAGS) -c chip8.c

ops.o: ops.c ops.h
	$(CC) $(CFLAGS) -c ops.c

chocolate_chip: run.c chip8.o ops.o ui.c ui.h
	$(CC) $(CFLAGS) run.c chip8.o ops.o ui.c -o chocolate_chip $(LDFLAGS)

test: test_ops.c chip8.o ops.o ui.c
	$(CC) $(CFLAGS) -g test_ops.c chip8.o ops.o ui.c -o test $(LDFLAGS)
	./test

.PHONY: clean test

clean:
	rm -f *.o $(PROGS)
