CC = gcc
CFLAGS = -Wall -o
BIN = $(patsubst %.c,%,$(wildcard *.c))

.PHONY: all do

all: $(BIN) do

%: %.c
	$(CC) $(CFLAGS) $@ $^

do:
	echo -e '\033[31mRUN THE PROGRAM!!!\033[0m'
	./signal
