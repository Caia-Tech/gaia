CC = gcc
CFLAGS = -Wall -Wextra -O2

# Object files for modular system
OBJS = gate_types.o basic_gates.o memory_gates_modular.o adaptive_gates.o network_builder.o

# All targets
all: binary_gates experiments test_suite memory_gates test_modular demo_learning test_networks text_processor

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

# Text processor
text_processor: text_processor.c $(OBJS)
	$(CC) $(CFLAGS) -o text_processor text_processor.c $(OBJS)

# Text processor tests
test_text_processor: test_text_processor.c $(OBJS)
	$(CC) $(CFLAGS) -o test_text_processor test_text_processor.c $(OBJS)

# Advanced text processor
text_processor_advanced: text_processor_advanced.c $(OBJS)
	$(CC) $(CFLAGS) -o text_processor_advanced text_processor_advanced.c $(OBJS)

# Coherent text processor
text_coherence: text_coherence.c $(OBJS)
	$(CC) $(CFLAGS) -o text_coherence text_coherence.c $(OBJS)

# Text coherence V2
text_coherence_v2: text_coherence_v2.c $(OBJS)
	$(CC) $(CFLAGS) -o text_coherence_v2 text_coherence_v2.c $(OBJS) -lm

# Simple coherence V3
text_coherence_v3_simple: text_coherence_v3_simple.c $(OBJS)
	$(CC) $(CFLAGS) -o text_coherence_v3_simple text_coherence_v3_simple.c $(OBJS)

# Coherence proof
coherence_proof: coherence_proof.c $(OBJS)
	$(CC) $(CFLAGS) -o coherence_proof coherence_proof.c $(OBJS)

# Text training system
text_training_system: text_training_system.c $(OBJS)
	$(CC) $(CFLAGS) -o text_training_system text_training_system.c $(OBJS)

# Interactive text processor
text_interactive: text_interactive.c $(OBJS)
	$(CC) $(CFLAGS) -o text_interactive text_interactive.c $(OBJS)

# Train gaia system
train_gaia: train_gaia.c $(OBJS)
	$(CC) $(CFLAGS) -o train_gaia train_gaia.c $(OBJS)

# Gaia chat system
gaia_chat: gaia_chat.c $(OBJS)
	$(CC) $(CFLAGS) -o gaia_chat gaia_chat.c $(OBJS)

# Test framework
test_framework: test_framework.c gaia_chat.c $(OBJS)
	$(CC) $(CFLAGS) -o test_framework test_framework.c gaia_chat.c $(OBJS)

# Iterative trainer
iterative_trainer: iterative_trainer.c gaia_chat.c $(OBJS)
	$(CC) $(CFLAGS) -o iterative_trainer iterative_trainer.c gaia_chat.c $(OBJS)

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
	rm -f binary_gates experiments test_suite memory_gates test_modular demo_learning test_networks text_processor *.o

.PHONY: all run run_all run_modular clean