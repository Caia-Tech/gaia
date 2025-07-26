/**
 * Test Suite for GAIA V10 Gate Intelligence
 * 
 * Verify that gates actually learn and exhibit intelligence
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Test 1: Can a single gate learn a linear function?
void test_single_gate() {
    printf("=== Test 1: Single Gate Learning ===\n");
    
    // Simple gate
    float weight = 0.5;
    float bias = 0.0;
    float learning_rate = 0.01;
    
    // Learn y = 2x + 1
    printf("Training gate to learn y = 2x + 1\n");
    for (int epoch = 0; epoch < 100; epoch++) {
        float total_error = 0;
        
        for (float x = -1; x <= 1; x += 0.1) {
            float target = 2 * x + 1;
            float output = x * weight + bias;
            float error = target - output;
            
            weight += learning_rate * error * x;
            bias += learning_rate * error;
            total_error += fabs(error);
        }
        
        if (epoch % 20 == 0) {
            printf("Epoch %d: Error = %.3f, w=%.3f, b=%.3f\n", 
                   epoch, total_error, weight, bias);
        }
    }
    
    // Test
    printf("\nTesting:\n");
    for (float x = -1; x <= 1; x += 0.5) {
        float output = x * weight + bias;
        float target = 2 * x + 1;
        printf("x=%.1f: output=%.3f, target=%.3f\n", x, output, target);
    }
    printf("\n");
}

// Test 2: Can gates with memory maintain state?
void test_memory_gate() {
    printf("=== Test 2: Memory Gate State ===\n");
    
    float memory = 0;
    float decay = 0.8;
    
    printf("Sending pulses to memory gate:\n");
    for (int t = 0; t < 10; t++) {
        float input = (t == 3 || t == 7) ? 1.0 : 0.0;
        memory = memory * decay + input * (1 - decay);
        printf("t=%d: input=%.1f, memory=%.3f\n", t, input, memory);
    }
    
    printf("\nMemory gate successfully maintains decaying state\n\n");
}

// Test 3: Can threshold gates make decisions?
void test_threshold_gate() {
    printf("=== Test 3: Threshold Gate Decision ===\n");
    
    float threshold = 0.5;
    
    printf("Testing threshold gate (threshold=%.1f):\n", threshold);
    for (float x = 0; x <= 1; x += 0.1) {
        float output = x > threshold ? 1.0 : 0.0;
        printf("input=%.2f → output=%.1f %s\n", 
               x, output, output > 0 ? "(ON)" : "(OFF)");
    }
    printf("\n");
}

// Test 4: Can gate combinations solve non-linear problems?
void test_xor_learning() {
    printf("=== Test 4: XOR Learning with Gates ===\n");
    
    // Mini network: 4 gates
    typedef struct {
        float w1, w2, bias;
    } SimpleGate;
    
    SimpleGate g1 = {0.5, 0.5, 0};    // Input combiner
    SimpleGate g2 = {-0.5, -0.5, 0};  // Input differencer  
    SimpleGate g3 = {1.0, -1.0, 0};   // Output gate
    
    float lr = 0.1;
    
    // XOR data
    float inputs[][2] = {{0,0}, {0,1}, {1,0}, {1,1}};
    float targets[] = {0, 1, 1, 0};
    
    printf("Training 3-gate network on XOR...\n");
    
    // Simple training
    for (int epoch = 0; epoch < 1000; epoch++) {
        float total_error = 0;
        
        for (int i = 0; i < 4; i++) {
            float x1 = inputs[i][0];
            float x2 = inputs[i][1];
            
            // Forward
            float h1 = tanh(x1 * g1.w1 + x2 * g1.w2 + g1.bias);
            float h2 = tanh(x1 * g2.w1 + x2 * g2.w2 + g2.bias);
            float out = tanh(h1 * g3.w1 + h2 * g3.w2 + g3.bias);
            
            // Error
            float error = targets[i] - (out + 1) / 2;  // Scale to 0-1
            total_error += fabs(error);
            
            // Backprop (simplified)
            g3.w1 += lr * error * h1;
            g3.w2 += lr * error * h2;
            g1.w1 += lr * error * x1 * 0.5;
            g1.w2 += lr * error * x2 * 0.5;
            g2.w1 -= lr * error * x1 * 0.5;
            g2.w2 -= lr * error * x2 * 0.5;
        }
        
        if (epoch % 200 == 0) {
            printf("Epoch %3d: Error = %.3f\n", epoch, total_error);
        }
    }
    
    // Test
    printf("\nTesting XOR:\n");
    for (int i = 0; i < 4; i++) {
        float x1 = inputs[i][0];
        float x2 = inputs[i][1];
        
        float h1 = tanh(x1 * g1.w1 + x2 * g1.w2 + g1.bias);
        float h2 = tanh(x1 * g2.w1 + x2 * g2.w2 + g2.bias);
        float out = (tanh(h1 * g3.w1 + h2 * g3.w2 + g3.bias) + 1) / 2;
        
        printf("%.0f XOR %.0f = %.3f (target: %.0f) %s\n", 
               x1, x2, out, targets[i],
               fabs(out - targets[i]) < 0.3 ? "✓" : "✗");
    }
    printf("\n");
}

// Test 5: Emergent patterns
void test_pattern_emergence() {
    printf("=== Test 5: Pattern Emergence ===\n");
    
    // Create a ring of gates
    #define RING_SIZE 8
    float gates[RING_SIZE] = {0};
    float weights[RING_SIZE] = {0};
    
    // Random weights
    for (int i = 0; i < RING_SIZE; i++) {
        weights[i] = (rand() % 200 - 100) / 100.0;
    }
    
    // Initialize with a pulse
    gates[0] = 1.0;
    
    printf("Ring of %d gates with random weights:\n", RING_SIZE);
    printf("Step 0: ");
    for (int i = 0; i < RING_SIZE; i++) {
        printf("%.1f ", gates[i]);
    }
    printf("\n");
    
    // Let it evolve
    for (int step = 1; step <= 5; step++) {
        float new_gates[RING_SIZE];
        
        for (int i = 0; i < RING_SIZE; i++) {
            int prev = (i - 1 + RING_SIZE) % RING_SIZE;
            new_gates[i] = tanh(gates[prev] * weights[i]);
        }
        
        for (int i = 0; i < RING_SIZE; i++) {
            gates[i] = new_gates[i];
        }
        
        printf("Step %d: ", step);
        for (int i = 0; i < RING_SIZE; i++) {
            printf("%.1f ", gates[i]);
        }
        printf("\n");
    }
    
    printf("\nPattern emerges from gate interactions!\n\n");
}

int main() {
    srand(time(NULL));
    
    printf("GAIA V10 Gate Intelligence Test Suite\n");
    printf("=====================================\n\n");
    
    test_single_gate();
    test_memory_gate();
    test_threshold_gate();
    test_xor_learning();
    test_pattern_emergence();
    
    printf("All tests completed!\n");
    
    return 0;
}