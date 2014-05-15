CC = gcc
CFLAGS = -Wall -I../../0-common
LFLAGS = -Wall -o
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all clean do

all: echosrv echocli clean do

echosrv: echosrv.o ../../0-common/common.o
	$(CC) $(LFLAGS) $@ $^

echocli: echocli.o ../../0-common/common.o
	$(CC) $(LFLAGS) $@ $^

clean:
	rm -f $(OBJS)

do:
	echo -e '\033[31mRUN THE PROGRAM!!!\033[0m'
