#include <stdio.h>
#include <stdint.h>
#include <string.h>

// A gate with memory
typedef struct {
    uint8_t state;      // Current state
    uint8_t memory;     // Previous state
} MemoryGate;

// Initialize a memory gate
void init_gate(MemoryGate* gate) {
    gate->state = 0;
    gate->memory = 0;
}

// Process with memory - output depends on current AND previous inputs
uint8_t process_memory_gate(MemoryGate* gate, uint8_t input) {
    // New state depends on input XOR memory
    gate->state = input ^ gate->memory;
    
    // Update memory for next time
    gate->memory = input;
    
    return gate->state;
}

// A pattern detector using memory gates
typedef struct {
    MemoryGate gates[4];
    uint8_t detected;
} PatternDetector;

// Initialize pattern detector
void init_detector(PatternDetector* pd) {
    for (int i = 0; i < 4; i++) {
        init_gate(&pd->gates[i]);
    }
    pd->detected = 0;
}

// Detect pattern: 1, 0, 1, 1
uint8_t detect_pattern(PatternDetector* pd, uint8_t input) {
    // Shift pattern through gates
    for (int i = 3; i > 0; i--) {
        pd->gates[i].state = pd->gates[i-1].state;
    }
    pd->gates[0].state = input;
    
    // Check if pattern matches
    if (pd->gates[0].state == 1 &&
        pd->gates[1].state == 1 &&
        pd->gates[2].state == 0 &&
        pd->gates[3].state == 1) {
        pd->detected = 1;
        return 1;
    }
    return 0;
}

int main() {
    printf("gaia Memory Gates Demo\n");
    printf("========================\n\n");
    
    // Test 1: Simple memory gate
    printf("Test 1: Memory Gate (output = input XOR previous)\n");
    printf("Input | Memory | Output\n");
    printf("------+--------+-------\n");
    
    MemoryGate mg;
    init_gate(&mg);
    
    uint8_t test_inputs[] = {1, 1, 0, 1, 0, 0, 1};
    for (int i = 0; i < 7; i++) {
        uint8_t prev_memory = mg.memory;
        uint8_t output = process_memory_gate(&mg, test_inputs[i]);
        printf("  %d   |   %d    |   %d\n", test_inputs[i], prev_memory, output);
    }
    
    // Test 2: Pattern detection
    printf("\nTest 2: Pattern Detector (looking for 1,0,1,1)\n");
    printf("Input sequence: ");
    
    PatternDetector pd;
    init_detector(&pd);
    
    uint8_t pattern_test[] = {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1};
    for (int i = 0; i < 11; i++) {
        printf("%d", pattern_test[i]);
        if (detect_pattern(&pd, pattern_test[i])) {
            printf(" <- PATTERN DETECTED!");
        }
        if (i < 10) printf(", ");
    }
    printf("\n");
    
    return 0;
}