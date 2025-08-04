# 项目配置
CC = gcc
CFLAGS = -Wall -O2 -mavx2
TARGET = sm3
SRCS = main.c sm3.c sm3_opt.c LenExtAtt.c
OBJS = $(SRCS:.c=.o)

MTTARGET = MerkleTree
MTSRCS = MerkleTree.c sm3.c MerkleTreeTest.c
MTOBJS = $(MTSRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@make clear
	./$(TARGET)


$(MTTARGET): $(MTOBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.mt.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

mt: $(MTTARGET)
	@make clear
	./$(MTTARGET)

clear:
	rm -f $(OBJS) $(MTOBJS)

clean:
	rm -f $(TARGET) $(OBJS) $(MTTARGET) $(MTOBJS)

.PHONY: all clean run
