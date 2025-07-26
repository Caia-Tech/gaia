# Computational Map: Minimal Gate Networks

## Objective

Systematically map what computational problems can be solved with N gates and M states per gate. This is the unexplored territory between single logic gates and neural networks.

## The Grid

```
Number of Gates →
     1    2    3    4    5    6    7    8    9    10   ...
S  ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
t 2│ ✓  │ ✓  │ ✓  │ ✓  │ ✓  │ ✓  │ ✓  │ ✓  │ ?  │ ?  │ Binary
a  ├────┼────┼────┼────┼────┼────┼────┼────┼────┼────┤
t 3│ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ Ternary
e  ├────┼────┼────┼────┼────┼────┼────┼────┼────┼────┤
s 4│ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ ?  │ Quaternary
↓  └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
```

## Detailed Results

## Critical Discovery

The results depend entirely on what primitive operations are available to gates!

### Scenario 1: Gates with Full Boolean Operations (AND, OR, XOR, NOT)

**1 Binary Gate**
- ✓ AND (using AND gate: AND(A,B))
- ✓ OR (using OR gate: OR(A,B))
- ✓ XOR (using XOR gate: XOR(A,B))
- ✓ NOT (using NOT gate: NOT(A))

**Key insight:** If gates have access to the operations we're testing, 1 gate solves everything!

### Scenario 2: Limited Gate Operations (PASS, NOT, MEMORY, XOR_WITH_MEMORY)

This is what we originally tested with our 8-gate networks.

**Minimum gates needed:**
- NOT: 1 gate (if NOT operation available)
- AND: Cannot be solved with these operations
- OR: Cannot be solved with these operations  
- XOR: 2 gates (PASS + XOR_MEM)

**Key insight:** The choice of primitive operations determines what's computable. Our original gates (PASS, NOT, MEM, XOR_MEM) cannot implement AND/OR at any scale!

### The Real Question

We've been asking "how many gates?" when we should ask "which primitive operations?"

With AND/OR/XOR/NOT primitives: 1 gate = universal
With PASS/NOT/MEM/XOR_MEM: Limited to XOR-like computations

## What We Know

### 8 Binary Gates (2 states)
- ✓ XOR (130 generations)
- ✓ 3-bit Parity (659 generations)  
- ✓ Sequence Detection "101" (0 generations)
- ⚠️ AND/OR (75% accuracy)
- ✗ 2-bit Addition (56% accuracy)

## Test Protocol

For each (gates, states) combination:

### 1. Core Problems
- **NOT** - Simplest negation
- **AND** - Basic conjunction
- **OR** - Basic disjunction  
- **XOR** - Non-linear boundary
- **3-bit Parity** - Multi-input XOR
- **2-bit Addition** - Requires carry
- **Sequence "101"** - Temporal pattern

### 2. Metrics to Record
- Solvable? (Yes/No/Partial)
- Generations to solve
- Success rate if partial
- Gate configuration that emerged
- Key insights

### 3. Testing Order

**Phase 1: Binary State Space**
- [ ] 1 gate, 2 states
- [ ] 2 gates, 2 states
- [ ] 3 gates, 2 states
- [ ] 4 gates, 2 states
- [ ] 5 gates, 2 states
- [ ] 6 gates, 2 states
- [ ] 7 gates, 2 states
- [x] 8 gates, 2 states ← We are here

**Phase 2: Find XOR Minimum**
- Work backwards from 8 gates to find minimum needed

**Phase 3: Ternary Exploration**
- [ ] 3 gates, 3 states
- [ ] 4 gates, 3 states
- [ ] 5 gates, 3 states
- Continue until capabilities match binary

## Expected Discoveries

1. **Phase Transitions** - Where new capabilities suddenly appear
2. **Optimal Ratios** - Best gates/states combinations
3. **Problem Difficulty Order** - Which problems need more resources
4. **State vs Gate Tradeoffs** - When is more states better than more gates?

## Test Implementation Plan

1. Create parameterized test framework
2. Vary number of gates (MAX_GATES)
3. Vary number of states (modify gate operations)
4. Run standard test suite
5. Record results in this grid
6. Look for patterns

## Why This Matters

- **Fills the gap** between logic gates and neural networks
- **Reveals minimal requirements** for computational tasks
- **Could inform** hardware design, biological understanding
- **Nobody has done this** systematically

---

Next step: Start with 1 gate, 2 states and work our way up...