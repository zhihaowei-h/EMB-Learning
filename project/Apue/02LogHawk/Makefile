# LogHawk Makefile
# 使用方式：
#   make          编译项目
#   make clean    清理编译产物
#   make run      编译并运行（需要 /var/log/syslog 读权限）
#   make test     编译并用模拟日志测试（无需 root 权限）

# 项目名称
TARGET   := loghawk
CC       := gcc
# 编译选项：-I./inc 包含头文件目录
CFLAGS   := -Wall -Wextra -g -I./inc
# 链接选项：如果用到多线程或数学库可以在此添加，如 -lpthread
LDFLAGS  := 

# 自动获取 src 目录下所有的 .c 文件
SRCS     := $(wildcard src/*.c)
# 将 SRCS 中的 .c 后缀替换为 .o，存放在 objs 目录下
OBJS     := $(SRCS:src/%.c=objs/%.o)

# 默认目标
.PHONY: all clean run test log

all: objs $(TARGET)

# 创建存放 .o 文件的目录
objs:
	@mkdir -p objs
	@mkdir -p output_logs

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo ""
	@echo "编译成功！生成目标: $(TARGET)"
	@echo ""

# 编译每个 .c 文件生成 .o 文件
# $< 代表第一个依赖文件 (.c)，$@ 代表目标文件 (.o)
objs/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 运行（需要 sudo 读取 /var/log/syslog）
run: all
	sudo ./$(TARGET)

# 测试模式
test: all
	@echo "=== 测试模式：模拟 syslog 日志 ==="
	@touch /tmp/test_syslog.log
	@echo "启动 LogHawk（后台运行）..."
	@./$(TARGET) & 
	@sleep 1
	@echo "写入模拟数据..."
	@for i in {1..5}; do \
		echo "Apr 21 10:00:0$$i node1 kernel: [INFO] Test message $$i" >> /tmp/test_syslog.log; \
		echo "Apr 21 10:00:0$$i node1 mysql: [ERROR] Fail $$i" >> /tmp/test_syslog.log; \
		sleep 0.2; \
	done
	@sleep 2
	@echo "--- 结果检查 ---"
	@ls -l output_logs/
	@pkill -f ./$(TARGET) || true
	@echo "测试完成"

# 清理
clean:
	rm -rf objs $(TARGET)
	rm -f /tmp/loghawk.offset /tmp/loghawk.pid /tmp/test_syslog.log
	@echo "清理完成"