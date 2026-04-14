CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -I./inc -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lpthread -lcrypt -lrt

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = iot_gateway

.PHONY: all clean run rund stop status restart

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) core

# 调试运行 [cite: 108]
run: $(TARGET)
	./$(TARGET) ./conf/iot_gateway.conf

# 守护进程运行 [cite: 110, 113]
rund: $(TARGET)
	./$(TARGET) ./conf/iot_gateway.conf -d

stop:
	killall $(TARGET) 2>/dev/null || true