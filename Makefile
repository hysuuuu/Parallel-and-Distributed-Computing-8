CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = balance

all: $(TARGET)

$(TARGET): balance.c
	$(CC) $(CFLAGS) -o $(TARGET) balance.c

clean:
	rm -f $(TARGET)

.PHONY: all clean