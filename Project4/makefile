# 项目配置
CC = gcc
CFLAGS = -Wall -O2
TARGET = sm3
SRCS = main.c sm3.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@make clear
	./$(TARGET)

clear:
	rm -f $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean run
