.PHONY: all do

%: %.c
	gcc -Wall -o $@ $<

all: endian do

endian: endian.c

do:
	echo -e '\033[32mEXECUTE THE BIN !!!\033[0m'
	./endian
