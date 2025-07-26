/**
 * GAIA V10 Minimal - Even Purer Gate Intelligence
 * 
 * What if intelligence is just gates learning to route signals?
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_GATES 8

typedef struct {
    float w;  // weight
    float m;  // memory
} Gate;

// The entire network
Gate gates[NUM_GATES];
float learn_rate = 0.1;

// Gate functions - each one line
float g_pass(float x, Gate* g) { return x * g->w; }
float g_amp(float x, Gate* g) { return tanh(x * g->w * 2); }
float g_mem(float x, Gate* g) { return (g->m = g->m * 0.8 + x * 0.2) * g->w; }
float g_thresh(float x, Gate* g) { return x * g->w > 0.5 ? 1.0 : -1.0; }

// The network: combine 8 gates to solve XOR
float network(float a, float b) {
    // Layer 1: Extract features
    float x1 = g_pass(a, &gates[0]);
    float x2 = g_pass(b, &gates[1]);
    float x3 = g_amp(a, &gates[2]);
    float x4 = g_amp(b, &gates[3]);
    
    // Layer 2: Combine
    float h1 = g_mem(x1 + x2, &gates[4]);
    float h2 = g_thresh(x3 - x4, &gates[5]);
    
    // Output: XOR logic emerges
    float y1 = g_amp(h1, &gates[6]);
    float y2 = g_pass(h2, &gates[7]);
    
    return tanh(y1 + y2);
}

// Learning: adjust gates based on error
void learn(float a, float b, float target) {
    float out = network(a, b);
    float err = target - out;
    
    // Simple learning: all gates adjust
    for (int i = 0; i < NUM_GATES; i++) {
        gates[i].w += learn_rate * err * (0.5 + (rand() % 100) / 200.0);
        gates[i].w = fmax(-2, fmin(2, gates[i].w)); // Bounds
    }
}

int main() {
    printf("GAIA V10 Minimal - %d gates learning XOR\n\n", NUM_GATES);
    
    // Initialize gates randomly
    for (int i = 0; i < NUM_GATES; i++) {
        gates[i].w = (rand() % 200 - 100) / 100.0;
        gates[i].m = 0;
    }
    
    // XOR training data
    float data[][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {1,1,0}};
    
    // Train
    printf("Training...\n");
    for (int epoch = 0; epoch < 500; epoch++) {
        float total_err = 0;
        for (int i = 0; i < 4; i++) {
            learn(data[i][0], data[i][1], data[i][2] * 2 - 1); // Scale to -1,1
            total_err += fabs(data[i][2] - (network(data[i][0], data[i][1]) + 1) / 2);
        }
        if (epoch % 100 == 0) {
            printf("Epoch %3d: Error %.3f\n", epoch, total_err);
        }
    }
    
    // Test
    printf("\nResults:\n");
    for (int i = 0; i < 4; i++) {
        float out = network(data[i][0], data[i][1]);
        printf("%.0f XOR %.0f = %.3f (target: %.0f)\n", 
               data[i][0], data[i][1], (out + 1) / 2, data[i][2]);
    }
    
    // Show gate evolution
    printf("\nGate weights after learning:\n");
    for (int i = 0; i < NUM_GATES; i++) {
        printf("Gate %d: %.3f\n", i, gates[i].w);
    }
    
    // Demonstrate memory gate persistence
    printf("\nMemory gate test:\n");
    gates[4].m = 0; // Reset memory
    for (int i = 0; i < 5; i++) {
        float out = network(i == 2 ? 1 : 0, 0);
        printf("Step %d: %.3f (memory: %.3f)\n", i, out, gates[4].m);
    }
    
    return 0;
}