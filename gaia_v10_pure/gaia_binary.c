/**
 * GAIA V10 Binary - Strict Binary Gate Network
 * 
 * Rules:
 * 1. All gates output exactly 0 or 1
 * 2. No floating point except for weights
 * 3. Simple, deterministic behavior
 * 4. Maximum 10 gates
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_GATES 10
#define INPUTS 2
#define OUTPUTS 1

typedef struct {
    float weight;      // Connection strength
    int threshold;     // Activation threshold (0 or 1)
    int memory;        // Binary memory (0 or 1)
    int type;          // Gate type (0-3)
} BinaryGate;

typedef struct {
    BinaryGate gates[MAX_GATES];
    int connections[MAX_GATES][MAX_GATES];  // Binary connection matrix
    int num_gates;
} BinaryNetwork;

// Strict binary gate operations
int gate_and(int a, int b) { return a & b; }
int gate_or(int a, int b) { return a | b; }
int gate_xor(int a, int b) { return a ^ b; }
int gate_not(int a) { return !a; }

// Process a gate based on its type
int process_gate(BinaryGate* g, int input) {
    switch(g->type) {
        case 0: return input;                    // Pass through
        case 1: return gate_not(input);          // NOT
        case 2: g->memory = input; return input; // Memory
        case 3: return input ^ g->memory;        // XOR with memory
    }
    return input;
}

// Forward pass through network
int forward_binary(BinaryNetwork* net, int a, int b) {
    int values[MAX_GATES] = {0};
    
    // Input layer
    values[0] = a;
    values[1] = b;
    
    // Process each gate
    for (int i = 2; i < net->num_gates; i++) {
        int sum = 0;
        
        // Collect inputs from connected gates
        for (int j = 0; j < i; j++) {
            if (net->connections[j][i]) {
                sum += values[j];
            }
        }
        
        // Binary threshold
        int input = (sum >= net->gates[i].threshold) ? 1 : 0;
        values[i] = process_gate(&net->gates[i], input);
    }
    
    return values[net->num_gates - 1];  // Last gate is output
}

// Evolve network by flipping bits
void mutate_binary(BinaryNetwork* net) {
    int choice = rand() % 3;
    
    switch(choice) {
        case 0: { // Flip a connection
            int from = rand() % (net->num_gates - 1);
            int to = from + 1 + rand() % (net->num_gates - from - 1);
            net->connections[from][to] ^= 1;
            break;
        }
        case 1: { // Change gate type
            int gate = 2 + rand() % (net->num_gates - 2);
            net->gates[gate].type = rand() % 4;
            break;
        }
        case 2: { // Flip threshold
            int gate = 2 + rand() % (net->num_gates - 2);
            net->gates[gate].threshold ^= 1;
            break;
        }
    }
}

// Test XOR performance
int test_xor(BinaryNetwork* net) {
    int correct = 0;
    int tests[][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {1,1,0}};
    
    for (int i = 0; i < 4; i++) {
        int out = forward_binary(net, tests[i][0], tests[i][1]);
        if (out == tests[i][2]) correct++;
    }
    
    return correct;
}

int main() {
    srand(time(NULL));
    printf("GAIA V10 Binary - Pure Binary Gate Evolution\n");
    printf("============================================\n\n");
    
    // Initialize network
    BinaryNetwork net = {0};
    net.num_gates = 8;  // 2 inputs + 5 hidden + 1 output
    
    // Random initialization
    for (int i = 0; i < net.num_gates; i++) {
        net.gates[i].type = rand() % 4;
        net.gates[i].threshold = rand() % 2;
        net.gates[i].memory = 0;
    }
    
    // Random connections (forward only)
    for (int i = 0; i < net.num_gates - 1; i++) {
        for (int j = i + 1; j < net.num_gates; j++) {
            net.connections[i][j] = rand() % 2;
        }
    }
    
    // Evolution loop
    printf("Evolving binary network to solve XOR...\n");
    int best_score = 0;
    BinaryNetwork best_net = net;
    
    for (int gen = 0; gen < 10000; gen++) {
        // Test current network
        int score = test_xor(&net);
        
        if (score > best_score) {
            best_score = score;
            best_net = net;
            printf("Generation %4d: Score %d/4\n", gen, score);
            
            if (score == 4) {
                printf("\nSolved XOR!\n");
                break;
            }
        }
        
        // Save state
        BinaryNetwork saved = net;
        
        // Mutate
        mutate_binary(&net);
        
        // Test mutation
        if (test_xor(&net) < score) {
            net = saved;  // Revert if worse
        }
    }
    
    // Show solution
    net = best_net;
    printf("\nFinal network test:\n");
    printf("0 XOR 0 = %d\n", forward_binary(&net, 0, 0));
    printf("0 XOR 1 = %d\n", forward_binary(&net, 0, 1));
    printf("1 XOR 0 = %d\n", forward_binary(&net, 1, 0));
    printf("1 XOR 1 = %d\n", forward_binary(&net, 1, 1));
    
    // Show network structure
    printf("\nNetwork structure:\n");
    for (int i = 2; i < net.num_gates; i++) {
        printf("Gate %d (type %d, threshold %d): ", i, net.gates[i].type, net.gates[i].threshold);
        for (int j = 0; j < i; j++) {
            if (net.connections[j][i]) printf("%d ", j);
        }
        printf("\n");
    }
    
    return 0;
}