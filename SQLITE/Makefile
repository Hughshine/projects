CC := gcc
CFLAGS := -Wall -Wextra -pedantic -std=c99
TARGET := db

main: main.c
	$(CC) *.c -o $(TARGET)

.PHONY: test clean

test: main
	./db ./database/db.msqlite

clean: 
	rm -rf *.o $(TARGET)