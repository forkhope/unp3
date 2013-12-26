CC = gcc
CFLAGS = -Wall -I../../0-common
LFLAGS = -Wall -o
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
BIN = daytimecli

.PHONY: all clean do

all: $(BIN) clean do

$(BIN): $(OBJS) ../../0-common/common.o
	$(CC) $(LFLAGS) $@ $^

clean:
	rm -f $(OBJS)

# 下面的210.0.235.14是网上找到的香港授时中心官方时间服务器ip地址
do:
	echo -e '\033[31mRUN THE PROGRAM!!!\033[0m'
