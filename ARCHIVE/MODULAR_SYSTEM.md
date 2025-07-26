# gaia Modular Gate System

*Note: This represents ongoing experimental development. All features are proof-of-concept implementations.*

## Overview

The modular gate system provides a flexible architecture for building and experimenting with gate-based AI networks. Gates can be dynamically created, connected, and evaluated to form complex computational structures.

## Architecture

### Core Components

1. **Gate Types** (`gate_types.h/c`)
   - Standardized interface for all gates
   - Dynamic registration system
   - Extensible architecture

2. **Basic Gates** (`basic_gates.c`)
   - AND, OR, XOR, NOT
   - NAND, NOR
   - BUFFER, ZERO, ONE

3. **Memory Gates** (`memory_gates_modular.c`)
   - LATCH - SR latch behavior
   - DELAY - D flip-flop behavior
   - COUNTER - Incremental counting
   - MEMORY_BANK - 8-bit addressable memory
   - ACCUMULATOR - Value accumulation

4. **Adaptive Gates** (`adaptive_gates.c`)
   - THRESHOLD - Perceptron-like learning
   - PATTERN - Pattern memorization
   - CONFIDENCE - Probabilistic decisions
   - ADAPTIVE_AND - Self-adjusting strictness

5. **Network Builder** (`network_builder.c`)
   - Create complex networks from gates
   - Save/load networks from files
   - Dynamic input setting and evaluation

## Usage

### Building a Simple Network

```c
// Create network
Network* net = network_create();

// Add gates
network_add_gate(net, "input_a", "ZERO");
network_add_gate(net, "input_b", "ZERO");
network_add_gate(net, "output", "AND");

// Connect gates
network_connect(net, "input_a", "output");
network_connect(net, "input_b", "output");

// Set inputs and evaluate
network_set_input(net, "input_a", 1);
network_set_input(net, "input_b", 0);
uint8_t result = network_evaluate_gate(net, "output");

// Save network
network_save(net, "my_network.gaia");
```

### Network File Format

```
# gaia Network Configuration
GATE input_a ZERO
GATE input_b ZERO
GATE output AND

CONNECT input_a output
CONNECT input_b output
```

### Learning Example

```c
// Create adaptive gate
Gate* learner = gate_create("THRESHOLD");

// Train with examples
uint8_t inputs[] = {1, 0};
uint8_t expected = 0;  // Teaching AND function
learner->type->update(learner, inputs, expected);
```

## Building and Running

```bash
# Build everything
make all

# Run modular system test
./test_modular

# Run learning demonstrations
./demo_learning

# Test network builder
./test_networks
```

## Example Networks

The system includes several example networks in the `networks/` directory:

- **xor_network.gaia** - XOR using only NAND gates
- **half_adder.gaia** - Binary half-adder circuit
- **counter.gaia** - Simple counter with clock input
- **learning_demo.gaia** - Network with adaptive gates

## Key Insights

1. **Modularity**: Any gate type can be added by implementing the GateType interface
2. **Composability**: Complex behaviors emerge from simple gate combinations
3. **Learning**: Multiple learning approaches without backpropagation
4. **Persistence**: Networks can be saved and loaded, preserving structure

## Future Directions

- Larger networks (1000+ gates)
- Automated configuration search
- Performance optimizations
- Visualization tools
- Real-world applications

## Technical Details

### Gate Interface

Every gate must implement:
- `evaluate()` - Compute output from inputs
- `init()` - Initialize gate state
- `cleanup()` - Free resources (optional)
- `update()` - Learning/adaptation (optional)
- `serialize()/deserialize()` - State persistence (optional)

### Performance

- Gate evaluation: O(1) per gate
- Network evaluation: O(n) where n = number of gates
- Memory usage: Minimal (bytes per gate)
- No heap allocation during evaluation

This modular system demonstrates that complex AI behaviors can emerge from simple, understandable components that anyone can build and experiment with.