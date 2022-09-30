CC=gcc
CFLAGS := -g -std=gnu11 -Wall -Wconversion -Wtype-limits -pedantic
VGFLAGS := --leak-check=full --show-leak-kinds=all --track-origins=yes
PROGS := test malloc

CFILES = *.c
HFILES = malloc.h

all: $(PROGS)

malloc: malloc.o

build: *.c
	$(CC) $(CFLAGS) -o test $(CFILES) -lm

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

clean:
	rm -f $(PROGS) *.o core vgcore.*

test: build
	./test

valgrind: build
	valgrind $(VGFLAGS) ./test


.PHONY: all clean format
