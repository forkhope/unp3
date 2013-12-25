CC = gcc
CFLAGS = -Wall -c -o
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

%.o: %.c
	$(CC) $(CFLAGS) $@ $^

.PHONY: all clean

all: $(OBJS)

clean:
	rm -f $(OBJS)
