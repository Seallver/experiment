# 项目配置
CC = gcc
CFLAGS = -Wall -O2 -mavx2  -w 
TARGET = sm3
SRCS = main.c sm3.c sm3_opt.c LenExtAtt.c
OBJS = $(SRCS:.c=.o)

MTTARGET = MerkleTree
MTSRCS = MerkleTree.c sm3.c MerkleTreeTest.c
MTOBJS = $(MTSRCS:.c=.o)

all: run

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@rm -f $(OBJS)
	@./$(TARGET)


$(MTTARGET): $(MTOBJS)
	@$(CC) $(CFLAGS) -o $@ $^

%.mt.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

mt: $(MTTARGET)
	@rm -f $(MTOBJS)
	@./$(MTTARGET)
	

clean:
	@rm -f $(TARGET) $(OBJS) $(MTTARGET) $(MTOBJS)

.PHONY: all clean run
