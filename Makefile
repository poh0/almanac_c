CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
LIBS=

almanac: main.c
	$(CC) $(CFLAGS) -o almanac main.c $(LIBS)
