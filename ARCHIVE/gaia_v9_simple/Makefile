# Makefile for GAIA V9 Simple
# Target: <1000 lines of actually useful AI

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lm

TARGET = gaia_simple
SRCS = gaia_simple.c memory.c learning.c calculate.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete!"
	@echo "Total lines of code:"
	@wc -l *.c *.h | tail -1

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) gaia_memory.dat

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@echo "Running basic tests..."
	@echo -e "2 plus 2\nquit" | ./$(TARGET)

.PHONY: all clean run test