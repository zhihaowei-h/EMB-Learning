# ================================================
# 项目名称：IoT Gateway System (路径修复版)
# ================================================

CC = gcc
# 增加 -I./inc 确保能找到头文件
CFLAGS = -Wall -Wextra -O2 -std=c99 -D_POSIX_C_SOURCE=200809L -I./inc
LDFLAGS = -lpthread -lrt -lcrypt

TARGET = iot_gateway

# 核心：定义源码都在 src 目录下
SRC_DIR = src
INC_DIR = inc

# 自动获取 src 目录下所有的 .c 文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
# 将 src/xxx.c 映射成根目录下的 xxx.o
OBJS = $(patsubst $(SRC_DIR)/%.c, %.o, $(SRCS))

all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build successful! Target: $(TARGET)"

# 编译规则：明确告诉 make 去 src 找 .c，在当前目录生成 .o
%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning..."
	rm -f *.o $(TARGET)
	@echo "Clean complete!"

.PHONY: all clean