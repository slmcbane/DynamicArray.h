CC=gcc
CFLAGS=-O0 -std=c99 -Wall -Wextra -pedantic

test: test_lib.c ../src/DynamicArray.h
	$(CC) $(CFLAGS) test_lib.c -I../src -o test.out
	./test.out