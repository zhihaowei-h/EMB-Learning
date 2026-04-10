CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = EFF
SRCS = main.c auth.c file.c dir.c stat.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c eff.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS)