CC=gcc
FLAGS=-O3 --pedantic -lncurses -Wall -Werror
PROGS=chocolate_chip test

all: $(PROGS)

chip8.o: chip8.c chip8.h font.h
	$(CC) $(FLAGS) -c chip8.c

ops.o: ops.c ops.h
	$(CC) $(FLAGS) -c ops.c

chocolate_chip: run.c chip8.o ops.o ui.c ui.h
	$(CC) $(FLAGS) run.c chip8.o ops.o ui.c -o chocolate_chip

test: test_ops.c chip8.o ops.o ui.c
	$(CC) $(FLAGS) -g test_ops.c chip8.o ops.o ui.c -o test

.PHONY: clean

clean:
	rm -f *.o $(PROGS)
