CC = gcc
CFLAGS = -Wall -o

.PHONY: all do

%: %.c
	$(CC) $(CFLAGS) $@ $<

all: sock_ntop do

do:
	echo -e '\033[31mRUN THE PROGRAM!!!\033[0m'
	./sock_ntop
