CC = gcc
CFLAGS = -Wall -I../../0-common
LFLAGS = -Wall -o
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
BIN = daytimesrv.out

.PHONY: all clean do

all: $(BIN) clean do

$(BIN): $(OBJS) ../../0-common/common.o
	$(CC) $(LFLAGS) $@ $^

clean:
	rm -f $(OBJS)

do:
	echo -e '\033[31mRUN THE PROGRAM!!!\033[0m'
	sudo ./daytimesrv.out
