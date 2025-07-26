#include "gate_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// External registration functions
void register_basic_gates(void);
void register_memory_gates(void);
void register_adaptive_gates(void);

// Helper to create input gates
Gate** create_input_gates(int n, uint8_t* values) {
    Gate** inputs = malloc(n * sizeof(Gate*));
    for (int i = 0; i < n; i++) {
        inputs[i] = gate_create(values[i] ? "ONE" : "ZERO");
    }
    return inputs;
}

void free_input_gates(Gate** inputs, int n) {
    for (int i = 0; i < n; i++) {
        gate_destroy(inputs[i]);
    }
    free(inputs);
}

// Demo 1: Teaching a THRESHOLD gate to learn AND function
void demo_learn_and() {
    printf("\n=== Teaching THRESHOLD Gate to Learn AND ===\n");
    
    Gate* threshold = gate_create("THRESHOLD");
    
    // Training data for AND
    uint8_t training_data[][3] = {
        {0, 0, 0},  // inputs, expected
        {0, 1, 0},
        {1, 0, 0},
        {1, 1, 1}
    };
    
    printf("Initial behavior:\n");
    for (int i = 0; i < 4; i++) {
        Gate** inputs = create_input_gates(2, training_data[i]);
        
        // Connect and evaluate
        for (int j = 0; j < 2; j++) {
            gate_connect(threshold, inputs[j]);
        }
        
        uint8_t output = gate_evaluate(threshold);
        printf("  %d AND %d = %d (expected %d)\n", 
               training_data[i][0], training_data[i][1], 
               output, training_data[i][2]);
        
        // Disconnect
        for (int j = 0; j < 2; j++) {
            gate_disconnect(threshold, inputs[j]);
        }
        gate_reset(threshold);
        
        free_input_gates(inputs, 2);
    }
    
    // Train for 20 epochs
    printf("\nTraining for 20 epochs...\n");
    for (int epoch = 0; epoch < 20; epoch++) {
        for (int i = 0; i < 4; i++) {
            Gate** inputs = create_input_gates(2, training_data[i]);
            
            // Connect
            for (int j = 0; j < 2; j++) {
                gate_connect(threshold, inputs[j]);
            }
            
            // Get input values for update
            uint8_t input_vals[2] = {training_data[i][0], training_data[i][1]};
            
            // Update (learn)
            if (threshold->type->update) {
                threshold->type->update(threshold, input_vals, training_data[i][2]);
            }
            
            // Disconnect
            for (int j = 0; j < 2; j++) {
                gate_disconnect(threshold, inputs[j]);
            }
            gate_reset(threshold);
            
            free_input_gates(inputs, 2);
        }
    }
    
    printf("\nAfter training:\n");
    int correct = 0;
    for (int i = 0; i < 4; i++) {
        Gate** inputs = create_input_gates(2, training_data[i]);
        
        // Connect and evaluate
        for (int j = 0; j < 2; j++) {
            gate_connect(threshold, inputs[j]);
        }
        
        uint8_t output = gate_evaluate(threshold);
        printf("  %d AND %d = %d (expected %d) %s\n", 
               training_data[i][0], training_data[i][1], 
               output, training_data[i][2],
               output == training_data[i][2] ? "✓" : "✗");
        
        if (output == training_data[i][2]) correct++;
        
        // Disconnect
        for (int j = 0; j < 2; j++) {
            gate_disconnect(threshold, inputs[j]);
        }
        gate_reset(threshold);
        
        free_input_gates(inputs, 2);
    }
    
    printf("Accuracy: %d/4 = %.1f%%\n", correct, correct * 25.0);
    
    gate_destroy(threshold);
}

// Demo 2: Pattern memorization
void demo_pattern_memory() {
    printf("\n=== Pattern Memorization ===\n");
    
    Gate* pattern = gate_create("PATTERN");
    
    // Teach it some 3-bit patterns
    printf("Teaching patterns:\n");
    uint8_t patterns[][4] = {
        {1, 0, 1, 1},  // Pattern 101 -> 1
        {0, 1, 1, 1},  // Pattern 011 -> 1
        {1, 1, 0, 0},  // Pattern 110 -> 0
        {0, 0, 0, 0},  // Pattern 000 -> 0
    };
    
    for (int i = 0; i < 4; i++) {
        printf("  Pattern %d%d%d -> %d\n", 
               patterns[i][0], patterns[i][1], patterns[i][2], patterns[i][3]);
        
        if (pattern->type->update) {
            pattern->type->update(pattern, patterns[i], patterns[i][3]);
        }
    }
    
    // Test recall
    printf("\nTesting recall:\n");
    for (int i = 0; i < 4; i++) {
        Gate** inputs = create_input_gates(3, patterns[i]);
        
        // Connect
        for (int j = 0; j < 3; j++) {
            gate_connect(pattern, inputs[j]);
        }
        
        uint8_t output = gate_evaluate(pattern);
        printf("  Pattern %d%d%d recalled as: %d (expected %d) %s\n",
               patterns[i][0], patterns[i][1], patterns[i][2],
               output, patterns[i][3],
               output == patterns[i][3] ? "✓" : "✗");
        
        // Disconnect
        for (int j = 0; j < 3; j++) {
            gate_disconnect(pattern, inputs[j]);
        }
        gate_reset(pattern);
        
        free_input_gates(inputs, 3);
    }
    
    // Test unknown pattern
    printf("\nTesting unknown pattern:\n");
    uint8_t unknown[] = {1, 0, 0};
    Gate** inputs = create_input_gates(3, unknown);
    
    for (int j = 0; j < 3; j++) {
        gate_connect(pattern, inputs[j]);
    }
    
    uint8_t output = gate_evaluate(pattern);
    printf("  Pattern 100 (unknown) -> %d\n", output);
    
    for (int j = 0; j < 3; j++) {
        gate_disconnect(pattern, inputs[j]);
    }
    free_input_gates(inputs, 3);
    
    gate_destroy(pattern);
}

// Demo 3: Adaptive AND learning its strictness
void demo_adaptive_and() {
    printf("\n=== Adaptive AND Gate ===\n");
    
    Gate* adaptive = gate_create("ADAPTIVE_AND");
    
    // We want it to learn: output 1 only if at least 2 of 3 inputs are 1
    printf("Teaching: Output 1 if at least 2 of 3 inputs are active\n\n");
    
    // Test cases
    uint8_t test_cases[][4] = {
        {0, 0, 0, 0},  // 0 active -> 0
        {1, 0, 0, 0},  // 1 active -> 0
        {0, 1, 0, 0},  // 1 active -> 0
        {1, 1, 0, 1},  // 2 active -> 1
        {1, 0, 1, 1},  // 2 active -> 1
        {0, 1, 1, 1},  // 2 active -> 1
        {1, 1, 1, 1},  // 3 active -> 1
    };
    
    // Initial test
    printf("Before training:\n");
    for (int i = 0; i < 7; i++) {
        Gate** inputs = create_input_gates(3, test_cases[i]);
        
        for (int j = 0; j < 3; j++) {
            gate_connect(adaptive, inputs[j]);
        }
        
        uint8_t output = gate_evaluate(adaptive);
        int active = test_cases[i][0] + test_cases[i][1] + test_cases[i][2];
        printf("  %d active inputs -> %d (expected %d)\n", 
               active, output, test_cases[i][3]);
        
        for (int j = 0; j < 3; j++) {
            gate_disconnect(adaptive, inputs[j]);
        }
        gate_reset(adaptive);
        
        free_input_gates(inputs, 3);
    }
    
    // Train
    printf("\nTraining...\n");
    for (int epoch = 0; epoch < 50; epoch++) {
        for (int i = 0; i < 7; i++) {
            Gate** inputs = create_input_gates(3, test_cases[i]);
            
            for (int j = 0; j < 3; j++) {
                gate_connect(adaptive, inputs[j]);
            }
            
            if (adaptive->type->update) {
                adaptive->type->update(adaptive, test_cases[i], test_cases[i][3]);
            }
            
            for (int j = 0; j < 3; j++) {
                gate_disconnect(adaptive, inputs[j]);
            }
            gate_reset(adaptive);
            
            free_input_gates(inputs, 3);
        }
    }
    
    // Final test
    printf("\nAfter training:\n");
    int correct = 0;
    for (int i = 0; i < 7; i++) {
        Gate** inputs = create_input_gates(3, test_cases[i]);
        
        for (int j = 0; j < 3; j++) {
            gate_connect(adaptive, inputs[j]);
        }
        
        uint8_t output = gate_evaluate(adaptive);
        int active = test_cases[i][0] + test_cases[i][1] + test_cases[i][2];
        printf("  %d active inputs -> %d (expected %d) %s\n", 
               active, output, test_cases[i][3],
               output == test_cases[i][3] ? "✓" : "✗");
        
        if (output == test_cases[i][3]) correct++;
        
        for (int j = 0; j < 3; j++) {
            gate_disconnect(adaptive, inputs[j]);
        }
        gate_reset(adaptive);
        
        free_input_gates(inputs, 3);
    }
    
    printf("Accuracy: %d/7 = %.1f%%\n", correct, correct * 100.0 / 7);
    
    gate_destroy(adaptive);
}

int main() {
    printf("gaia Learning Demonstration\n");
    printf("===========================\n");
    
    // Initialize
    gate_registry_init();
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    // Run demos
    demo_learn_and();
    demo_pattern_memory();
    demo_adaptive_and();
    
    printf("\n✓ Learning demonstrations complete!\n");
    printf("\nKey insights:\n");
    printf("- Gates can learn without backpropagation\n");
    printf("- Simple update rules achieve learning\n");
    printf("- Memory and adaptation emerge from basic operations\n");
    
    gate_registry_cleanup();
    return 0;
}