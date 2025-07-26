/**
 * GAIA V10 Emergence - Watch Intelligence Emerge from Gate Chaos
 * 
 * 16 gates. Random connections. See what emerges.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define GATES 16
#define INPUTS 4
#define OUTPUTS 2

// Minimal gate structure
typedef struct {
    float w[GATES];     // Weights to other gates
    float value;        // Current output
    float bias;         // Learnable bias
    int type;           // Gate type (0-3)
} Gate;

Gate network[GATES];

// Gate operations
float compute_gate(Gate* g, float input) {
    switch(g->type) {
        case 0: return tanh(input * 2 + g->bias);           // Amplifier
        case 1: return input > g->bias ? 1.0 : 0.0;         // Threshold
        case 2: g->value = g->value * 0.9 + input * 0.1;    // Memory
                return g->value + g->bias;
        case 3: return sin(input * 3.14159 + g->bias);      // Oscillator
    }
    return input;
}

// Forward pass - gates influence each other
void forward(float inputs[INPUTS]) {
    float new_values[GATES];
    
    // Input gates get external input
    for (int i = 0; i < INPUTS; i++) {
        new_values[i] = compute_gate(&network[i], inputs[i]);
    }
    
    // Hidden gates combine other gates
    for (int i = INPUTS; i < GATES; i++) {
        float sum = 0;
        for (int j = 0; j < i; j++) {  // Can only see previous gates
            sum += network[j].value * network[i].w[j];
        }
        new_values[i] = compute_gate(&network[i], sum);
    }
    
    // Update all values
    for (int i = 0; i < GATES; i++) {
        network[i].value = new_values[i];
    }
}

// Evolution: random mutations
void mutate() {
    int gate = rand() % GATES;
    int param = rand() % 3;
    
    switch(param) {
        case 0: // Mutate weight
            if (gate >= INPUTS) {  // Only hidden gates have weights
                int connection = rand() % gate;
                network[gate].w[connection] += (rand() % 100 - 50) / 50.0;
            }
            break;
        case 1: // Mutate bias
            network[gate].bias += (rand() % 100 - 50) / 100.0;
            break;
        case 2: // Mutate type
            network[gate].type = rand() % 4;
            break;
    }
}

// Fitness: can it recognize patterns?
float evaluate_fitness() {
    float fitness = 0;
    
    // Test pattern: output should match input parity
    for (int test = 0; test < 16; test++) {
        float inputs[INPUTS];
        int bits = 0;
        
        // Convert test number to binary inputs
        for (int i = 0; i < INPUTS; i++) {
            inputs[i] = (test >> i) & 1;
            bits += inputs[i];
        }
        
        forward(inputs);
        
        // Check if output matches parity
        float target = (bits % 2);
        fitness += 1 - fabs(target - network[GATES-1].value);
    }
    
    return fitness / 16.0;
}

int main() {
    srand(time(NULL));
    printf("GAIA V10 Emergence - Evolution of %d Gates\n\n", GATES);
    
    // Random initialization
    for (int i = 0; i < GATES; i++) {
        network[i].type = rand() % 4;
        network[i].bias = (rand() % 100 - 50) / 50.0;
        network[i].value = 0;
        for (int j = 0; j < i; j++) {
            network[i].w[j] = (rand() % 100 - 50) / 50.0;
        }
    }
    
    float best_fitness = 0;
    Gate best_network[GATES];
    
    // Evolution loop
    printf("Evolving...\n");
    for (int gen = 0; gen < 10000; gen++) {
        // Save current state
        Gate saved[GATES];
        for (int i = 0; i < GATES; i++) saved[i] = network[i];
        
        // Try mutation
        mutate();
        float fitness = evaluate_fitness();
        
        // Keep if better
        if (fitness > best_fitness) {
            best_fitness = fitness;
            for (int i = 0; i < GATES; i++) best_network[i] = network[i];
            
            if (gen % 1000 == 0) {
                printf("Gen %4d: Fitness %.3f\n", gen, fitness);
            }
        } else {
            // Restore
            for (int i = 0; i < GATES; i++) network[i] = saved[i];
        }
    }
    
    // Use best network
    for (int i = 0; i < GATES; i++) network[i] = best_network[i];
    printf("\nFinal fitness: %.3f\n", best_fitness);
    
    // Test the evolved network
    printf("\nTesting evolved behavior (should detect parity):\n");
    for (int test = 0; test < 8; test++) {
        float inputs[INPUTS] = {0};
        int bits = 0;
        
        for (int i = 0; i < 3; i++) {
            inputs[i] = (test >> i) & 1;
            bits += inputs[i];
            printf("%.0f", inputs[i]);
        }
        
        forward(inputs);
        printf(" → %.3f (parity: %d)\n", network[GATES-1].value, bits % 2);
    }
    
    // Show gate connectivity
    printf("\nEmergent gate structure:\n");
    for (int i = INPUTS; i < GATES - OUTPUTS; i++) {
        printf("Gate %2d (type %d):", i, network[i].type);
        int connections = 0;
        for (int j = 0; j < i; j++) {
            if (fabs(network[i].w[j]) > 0.5) {
                printf(" ←%d", j);
                connections++;
            }
        }
        if (connections == 0) printf(" (isolated)");
        printf("\n");
    }
    
    return 0;
}