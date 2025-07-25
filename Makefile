CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = caia

all: $(TARGET)

$(TARGET): binary_gates.c
	$(CC) $(CFLAGS) -o $(TARGET) binary_gates.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean