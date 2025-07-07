CC = gcc
CFLAGS = -Wall -O2 -maes -mssse3

TARGET = sm4_test
SRCS = main.c sm4.c sm4_aesni.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
