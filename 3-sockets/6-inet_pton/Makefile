CC = gcc
CFLAGS = -Wall -o

.PHONY: all do

%.out: %.c
	$(CC) $(CFLAGS) $@ $<

all: inet_pton.out do

do:
	echo -e '\033[31mRUN THE PROGRAM!!!\033[0m'
	./inet_pton.out
