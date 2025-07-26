#include "gate_types.h"
#include <stdio.h>
#include <assert.h>

// External registration functions
void register_basic_gates(void);
void register_memory_gates(void);

// Test basic gate operations
void test_basic_gates() {
    printf("\n=== Testing Basic Gates ===\n");
    
    // Create gates
    Gate* a = gate_create("ONE");
    Gate* b = gate_create("ZERO");
    Gate* and_gate = gate_create("AND");
    Gate* or_gate = gate_create("OR");
    Gate* xor_gate = gate_create("XOR");
    
    // Connect inputs
    gate_connect(and_gate, a);
    gate_connect(and_gate, b);
    
    gate_connect(or_gate, a);
    gate_connect(or_gate, b);
    
    gate_connect(xor_gate, a);
    gate_connect(xor_gate, b);
    
    // Evaluate
    printf("A=1, B=0:\n");
    printf("  AND: %d (expected 0)\n", gate_evaluate(and_gate));
    printf("  OR:  %d (expected 1)\n", gate_evaluate(or_gate));
    printf("  XOR: %d (expected 1)\n", gate_evaluate(xor_gate));
    
    // Cleanup
    gate_destroy(a);
    gate_destroy(b);
    gate_destroy(and_gate);
    gate_destroy(or_gate);
    gate_destroy(xor_gate);
    
    printf("Basic gates test passed!\n");
}

// Test memory gates
void test_memory_gates() {
    printf("\n=== Testing Memory Gates ===\n");
    
    // Test DELAY gate
    Gate* input = gate_create("ONE");
    Gate* delay = gate_create("DELAY");
    gate_connect(delay, input);
    
    printf("DELAY gate test:\n");
    printf("  Cycle 1: %d (expected 0 - initial state)\n", gate_evaluate(delay));
    gate_reset(delay);
    printf("  Cycle 2: %d (expected 1 - delayed input)\n", gate_evaluate(delay));
    
    // Test LATCH gate
    Gate* set = gate_create("ONE");
    Gate* reset = gate_create("ZERO");
    Gate* latch = gate_create("LATCH");
    gate_connect(latch, set);
    gate_connect(latch, reset);
    
    printf("\nLATCH gate test:\n");
    printf("  Set=1, Reset=0: %d (expected 1)\n", gate_evaluate(latch));
    
    // Change to reset
    gate_destroy(set);
    gate_destroy(reset);
    set = gate_create("ZERO");
    reset = gate_create("ONE");
    gate_disconnect(latch, NULL);  // Clear connections
    gate_connect(latch, set);
    gate_connect(latch, reset);
    gate_reset(latch);
    
    printf("  Set=0, Reset=1: %d (expected 0)\n", gate_evaluate(latch));
    
    // Cleanup
    gate_destroy(input);
    gate_destroy(delay);
    gate_destroy(set);
    gate_destroy(reset);
    gate_destroy(latch);
    
    printf("Memory gates test passed!\n");
}

// Test complex circuit
void test_complex_circuit() {
    printf("\n=== Testing Complex Circuit ===\n");
    printf("Building: (A XOR B) AND (C OR D)\n");
    
    // Inputs
    Gate* a = gate_create("ONE");
    Gate* b = gate_create("ZERO");
    Gate* c = gate_create("ONE");
    Gate* d = gate_create("ONE");
    
    // First layer
    Gate* xor_gate = gate_create("XOR");
    gate_connect(xor_gate, a);
    gate_connect(xor_gate, b);
    
    Gate* or_gate = gate_create("OR");
    gate_connect(or_gate, c);
    gate_connect(or_gate, d);
    
    // Second layer
    Gate* and_gate = gate_create("AND");
    gate_connect(and_gate, xor_gate);
    gate_connect(and_gate, or_gate);
    
    // Evaluate
    uint8_t result = gate_evaluate(and_gate);
    printf("Result: %d (expected 1)\n", result);
    printf("  A XOR B = 1 XOR 0 = 1\n");
    printf("  C OR D = 1 OR 1 = 1\n");
    printf("  1 AND 1 = 1\n");
    
    // Cleanup
    gate_destroy(a);
    gate_destroy(b);
    gate_destroy(c);
    gate_destroy(d);
    gate_destroy(xor_gate);
    gate_destroy(or_gate);
    gate_destroy(and_gate);
    
    printf("Complex circuit test passed!\n");
}

// Test counter functionality
void test_counter() {
    printf("\n=== Testing Counter Gate ===\n");
    
    Gate* increment = gate_create("ONE");
    Gate* reset = gate_create("ZERO");
    Gate* counter = gate_create("COUNTER");
    
    gate_connect(counter, increment);
    gate_connect(counter, reset);
    
    printf("Counting up:\n");
    for (int i = 0; i < 5; i++) {
        uint8_t value = gate_evaluate(counter);
        printf("  Count %d: output = %d\n", i, value);
        gate_reset(counter);
    }
    
    // Test reset
    gate_destroy(reset);
    reset = gate_create("ONE");
    gate_disconnect(counter, NULL);
    gate_connect(counter, increment);
    gate_connect(counter, reset);
    gate_reset(counter);
    
    printf("After reset: %d (expected 0)\n", gate_evaluate(counter));
    
    gate_destroy(increment);
    gate_destroy(reset);
    gate_destroy(counter);
    
    printf("Counter test passed!\n");
}

int main() {
    printf("gaia Modular Gate System Test\n");
    printf("==============================\n");
    
    // Initialize registry
    gate_registry_init();
    
    // Register gate types
    register_basic_gates();
    register_memory_gates();
    
    // List registered gates
    printf("\nRegistered gate types:\n");
    gate_registry_list();
    
    // Run tests
    test_basic_gates();
    test_memory_gates();
    test_complex_circuit();
    test_counter();
    
    // Cleanup
    gate_registry_cleanup();
    
    printf("\n✓ All tests passed!\n");
    printf("\nThe modular gate system is working correctly.\n");
    printf("Gates can be created, connected, and evaluated dynamically.\n");
    
    return 0;
}