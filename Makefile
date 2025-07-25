CC = gcc
CFLAGS = -Wall -Wextra -O2

# Object files for modular system
OBJS = gate_types.o basic_gates.o memory_gates_modular.o adaptive_gates.o network_builder.o

# All targets
all: binary_gates experiments test_suite memory_gates test_modular demo_learning test_networks

# Original demos
binary_gates: binary_gates.c
	$(CC) $(CFLAGS) -o binary_gates binary_gates.c

experiments: experiments.c
	$(CC) $(CFLAGS) -o experiments experiments.c

test_suite: test_suite.c
	$(CC) $(CFLAGS) -o test_suite test_suite.c

memory_gates: memory_gates.c
	$(CC) $(CFLAGS) -o memory_gates memory_gates.c

# Modular system components
gate_types.o: gate_types.c gate_types.h
	$(CC) $(CFLAGS) -c gate_types.c

basic_gates.o: basic_gates.c gate_types.h
	$(CC) $(CFLAGS) -c basic_gates.c

memory_gates_modular.o: memory_gates_modular.c gate_types.h
	$(CC) $(CFLAGS) -c memory_gates_modular.c

adaptive_gates.o: adaptive_gates.c gate_types.h
	$(CC) $(CFLAGS) -c adaptive_gates.c

network_builder.o: network_builder.c gate_types.h
	$(CC) $(CFLAGS) -c network_builder.c

# Modular system test
test_modular: test_modular.c $(OBJS)
	$(CC) $(CFLAGS) -o test_modular test_modular.c $(OBJS)

# Network builder test
test_networks: test_networks.c $(OBJS)
	$(CC) $(CFLAGS) -o test_networks test_networks.c $(OBJS)

# Learning demonstration
demo_learning: demo_learning.c $(OBJS)
	$(CC) $(CFLAGS) -o demo_learning demo_learning.c $(OBJS) -lm

# Run targets
run: binary_gates
	./binary_gates

run_all: all
	@echo "Running original binary gates demo..."
	./binary_gates
	@echo "\nRunning experiments..."
	./experiments
	@echo "\nRunning test suite..."
	./test_suite
	@echo "\nRunning modular system test..."
	./test_modular

run_modular: test_modular
	./test_modular

# Clean
clean:
	rm -f binary_gates experiments test_suite memory_gates test_modular demo_learning test_networks *.o

.PHONY: all run run_all run_modular clean