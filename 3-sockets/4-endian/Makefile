.PHONY: all do

%.out: %.c
	gcc -Wall -o $@ $<

all: endian.out do

endian.out: endian.c

do:
	echo -e '\033[32mEXECUTE THE BIN !!!\033[0m'
	./endian.out
