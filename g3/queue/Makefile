.PHONY: all clean valgrind quickgrind debug

CC=gcc
CFLAGS=-Wall -Werror -Wextra -pedantic -std=c11 -g -pthread

all: bouncer test

bouncer: queue.o bouncer.c
	$(CC) $(CFLAGS) -o bouncer queue.o bouncer.c

queue.o: queue.c
	$(CC) $(CFLAGS) -c queue.c

test: queue.o
	$(CC) $(CFLAGS) -o test queue.o test.c

clean:
	rm -f *.o
	rm -f bouncer

valgrind: bouncer
	valgrind --track-origins=yes --leak-check=full ./bouncer

quickgrind: bouncer
	cat input.txt | valgrind --track-origins=yes --leak-check=full ./bouncer

debug: input.txt gdb.script bouncer
	gdb -q -x gdb.script ./bouncer
