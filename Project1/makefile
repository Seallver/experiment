CC = gcc
CFLAGS = -Wall -maes -msse4 -s

TARGET = sm4_test
SRCS = main.c sm4.c sm4_aesni.c sm4_ttable.c
OBJS = $(SRCS:.c=.o)

BENCHMARK_TARGET = benchmark
BENCHMARK_SRCS = benchmark.c sm4.c sm4_aesni.c sm4_ttable.c
BENCHMARK_OBJS = $(BENCHMARK_SRCS:.c=.o)

GCM_TARGET = sm4_gcm
GCM_SRCS = SM4_GCM/sm4_gcm.c sm4.c SM4_GCM/sm4_gcm_test.c SM4_GCM/ghash.c
GCM_OBJS = $(GCM_SRCS:.c=.o)

# 默认目标：构建 sm4_test 并运行
all: $(TARGET)
	@rm -f $(OBJS) $(BENCHMARK_OBJS)
	@echo "执行 sm4_test:"
	./$(TARGET)

# 构建 sm4_test
$(TARGET): CFLAGS += -Ofast
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 构建 benchmark 并运行
bm: $(BENCHMARK_TARGET) 
	@rm -f $(OBJS) $(BENCHMARK_OBJS)
	@echo "执行 benchmark:"
	./$(BENCHMARK_TARGET)

# 编译 benchmark 使用 -O0
$(BENCHMARK_TARGET): CFLAGS += -Ofast
$(BENCHMARK_TARGET): $(BENCHMARK_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 构建 gcm 并运行
gcm: $(GCM_TARGET) 
	@rm -f $(OBJS) $(GCM_OBJS)
	@echo "执行 sm4_gcm_test:"
	./$(GCM_TARGET)

# 编译 gcm 使用 -O0
$(GCM_TARGET): CFLAGS += -Ofast
$(GCM_TARGET): $(GCM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^


# 清理所有输出文件
clean:
	rm -f $(OBJS) $(TARGET) $(BENCHMARK_OBJS)  $(BENCHMARK_TARGET) $(GCM_OBJS) $(GCM_TARGET)

.PHONY: all clean benchmark clear

