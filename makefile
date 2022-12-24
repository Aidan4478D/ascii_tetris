# Definitions for constants
CC=gcc
CFLAGS=-I.

.PHONY: clean

all: make clean

# This will create your final output using .o compiled files
make: main.o board_logic.o
	$(CC) $(CFLAGS) -o main main.o board_logic.o -lncurses

board_logic.o: board_logic.c board_logic.h
	$(CC) $(CFLAGS) -c board_logic.c -lncurses

# This will compile main.c with its dependency
main.o: main.c board_logic.h
	$(CC) $(CFLAGS) -c main.c -lncurses

# This will clean or remove compiled files so you can start fresh
clean:
	rm -f *.o *.stackdump tetris 









