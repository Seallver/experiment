CC = gcc
CFLAGS = -Wall -maes -msse4 -s

TARGET = sm4_test
SRCS = main.c sm4.c sm4_aesni.c sm4_opt.c
OBJS = $(SRCS:.c=.o)

BENCHMARK_TARGET = benchmark
BENCHMARK_SRCS = benchmark.c sm4.c sm4_aesni.c sm4_opt.c
BENCHMARK_OBJS = $(BENCHMARK_SRCS:.c=.o)

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

# 清理所有输出文件
clean:
	rm -f $(OBJS) $(BENCHMARK_OBJS) $(TARGET) $(BENCHMARK_TARGET)

.PHONY: all clean benchmark clear

