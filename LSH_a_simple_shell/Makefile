CC := gcc
CFLAGS := -Wall -Wextra -pedantic -std=c99
TARGET := lsh

main: main.c
	$(CC) *.c -o lsh 

.PHONY: test clean

test: main
	./lsh

clean: 
	rm -rf *.o $(TARGET)