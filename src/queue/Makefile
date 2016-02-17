.PHONY: all clean valgrind quickgrind debug

CC=gcc
CFLAGS=-Werror -Wextra -pedantic -std=c11 -g

all: bouncer

bouncer: bouncer.c
	$(CC) $(CFLAGS) -o bouncer bouncer.c

clean:
	rm -f *.o
	rm -f bouncer

valgrind: bouncer
	valgrind --track-origins=yes --leak-check=full ./bouncer

quickgrind: bouncer
	cat input.txt | valgrind --track-origins=yes --leak-check=full ./bouncer

debug: input.txt gdb.script bouncer
	gdb -q -x gdb.script ./bouncer
