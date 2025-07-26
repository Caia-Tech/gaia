# GAIA MCP Server

An MCP (Model Context Protocol) server for experimenting with GAIA logic gate architectures, superposition states, and adaptive circuit topologies.

## Features

### Circuit Types

1. **Basic Circuits** - Standard logic gate arrangements (AND, OR, NOT, XOR, NAND, NOR, THRESHOLD)
2. **Superposition Circuits** - Gates that maintain quantum-like superposition states until collapsed
3. **Temporal Circuits** - Gates with memory that can delay, integrate, differentiate, or detect patterns
4. **Adaptive Circuits** - Self-modifying topologies that evolve based on fitness functions

### Available Tools

- `create_circuit` - Create new circuits of any type
- `execute_circuit` - Run circuits with specific inputs
- `visualize_circuit` - Get visual representation of circuit structure
- `observe_superposition` - View probability distributions in superposition circuits
- `evolve_circuit` - Evolve adaptive circuits using custom fitness functions

## Installation

```bash
cd mcp-server
npm install
npm run build
```

## Usage

### As MCP Server

Add to your Claude Desktop configuration:

```json
{
  "mcpServers": {
    "gaia": {
      "command": "node",
      "args": ["/path/to/gaia/mcp-server/dist/index.js"]
    }
  }
}
```

### Example: Superposition Circuit

```typescript
// Create a superposition circuit
create_circuit({
  name: "quantum_and",
  type: "superposition",
  config: {
    inputCount: 2,
    outputCount: 1
  }
})

// Execute and observe superposition
execute_circuit({
  name: "quantum_and",
  inputs: [1, 1]
})

// View quantum states
observe_superposition({
  name: "quantum_and"
})
```

### Example: Evolving XOR Circuit

```typescript
// Create adaptive circuit
create_circuit({
  name: "evolving_xor",
  type: "adaptive",
  config: {
    inputCount: 2,
    outputCount: 1,
    mutationRate: 0.2
  }
})

// Evolve to learn XOR
evolve_circuit({
  name: "evolving_xor",
  fitness_function: `
    // XOR truth table fitness
    const expected = [
      [[0,0], [0]],
      [[0,1], [1]], 
      [[1,0], [1]],
      [[1,1], [0]]
    ];
    let correct = 0;
    for (const [input, output] of expected) {
      if (JSON.stringify(inputs) === JSON.stringify(input) &&
          JSON.stringify(outputs) === JSON.stringify(output)) {
        correct++;
      }
    }
    return correct / expected.length;
  `,
  iterations: 500
})
```

## Research Areas

### Superposition Mechanics
- How do logic gates behave in superposition?
- Can deterministic systems exhibit quantum-like properties?
- What emerges from probabilistic gate networks?

### Temporal Patterns
- How does memory change circuit behavior?
- Can circuits learn temporal sequences?
- What patterns emerge from feedback loops?

### Adaptive Evolution
- Can circuits discover optimal topologies?
- How fast can logic gates learn complex functions?
- What novel architectures emerge from evolution?

### Composability
- How do different circuit types interact?
- Can we build circuits of circuits?
- What happens at massive scales?

## Contributing

Document your experiments! Each novel architecture discovered helps expand our understanding of composable intelligence.