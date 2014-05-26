CC = gcc
CFLAGS = -Wall -o
BIN = $(patsubst %.c,%.out,$(wildcard *.c))

.PHONY: all do

all: $(BIN) do

%.out: %.c
	$(CC) $(CFLAGS) $@ $^

do:
	echo -e '\033[31mRUN THE PROGRAM!!!\033[0m'
	./signal.out
