/**
 * Systematic Test: What can N gates compute?
 * Starting with 1 gate and working up
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_GENERATIONS 1000

typedef struct {
    int type;      // Gate type
    float weight;  // Weight/threshold
    int memory;    // Memory state
} Gate;

typedef struct {
    Gate* gates;
    int num_gates;
    int** connections;  // Dynamic connection matrix
} Network;

// Gate operations (binary for now)
int apply_gate(Gate* g, int input) {
    switch(g->type) {
        case 0: return input;                    // PASS
        case 1: return !input;                   // NOT
        case 2: g->memory = input; return input; // MEMORY
        case 3: return input ^ g->memory;        // XOR_MEM
    }
    return input;
}

// Create network with N gates
Network* create_network(int n) {
    Network* net = malloc(sizeof(Network));
    net->num_gates = n;
    net->gates = calloc(n, sizeof(Gate));
    
    // Allocate connection matrix
    net->connections = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        net->connections[i] = calloc(n, sizeof(int));
    }
    
    return net;
}

void free_network(Network* net) {
    for (int i = 0; i < net->num_gates; i++) {
        free(net->connections[i]);
    }
    free(net->connections);
    free(net->gates);
    free(net);
}

// Forward pass for 1-gate network
int forward_1gate(Network* net, int a, int b) {
    // 1 gate cannot see both inputs separately
    // It can only see one combined value
    // Options: a+b, a*b, a|b, a&b, etc.
    // For now: use OR combination (a|b)
    int combined = a | b;
    return apply_gate(&net->gates[0], combined);
}

// Forward pass for 2-gate network  
int forward_2gate(Network* net, int a, int b) {
    int val0 = apply_gate(&net->gates[0], a);
    int val1 = apply_gate(&net->gates[1], b);
    
    // Output could be from either gate or combination
    if (net->connections[0][1]) {
        return val0 ^ val1;  // XOR combination
    }
    return val1;  // Default to second gate
}

// Forward pass for N-gate network
int forward_ngate(Network* net, int a, int b) {
    int* values = calloc(net->num_gates, sizeof(int));
    
    // Input layer (first 2 gates get inputs)
    if (net->num_gates >= 1) values[0] = apply_gate(&net->gates[0], a);
    if (net->num_gates >= 2) values[1] = apply_gate(&net->gates[1], b);
    
    // Process remaining gates
    for (int i = 2; i < net->num_gates; i++) {
        int input = 0;
        int count = 0;
        
        // Collect inputs from connected gates
        for (int j = 0; j < i; j++) {
            if (net->connections[j][i]) {
                input ^= values[j];  // XOR inputs together
                count++;
            }
        }
        
        // If no connections, take from previous gate
        if (count == 0 && i > 0) {
            input = values[i-1];
        }
        
        values[i] = apply_gate(&net->gates[i], input);
    }
    
    int result = values[net->num_gates - 1];
    free(values);
    return result;
}

// Test NOT
int test_not(Network* net) {
    if (net->num_gates == 1) {
        int out0 = apply_gate(&net->gates[0], 0);
        int out1 = apply_gate(&net->gates[0], 1);
        return (out0 == 1 && out1 == 0) ? 2 : 0;
    }
    
    // For multi-gate, test each input
    int correct = 0;
    int tests[][2] = {{0,1}, {1,0}};
    
    for (int i = 0; i < 2; i++) {
        int out = forward_ngate(net, tests[i][0], 0);
        if (out == tests[i][1]) correct++;
    }
    return correct;
}

// Test AND
int test_and(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,0}, {1,0,0}, {1,1,1}};
    int correct = 0;
    
    for (int i = 0; i < 4; i++) {
        int out;
        if (net->num_gates == 1) {
            out = forward_1gate(net, tests[i][0], tests[i][1]);
        } else if (net->num_gates == 2) {
            out = forward_2gate(net, tests[i][0], tests[i][1]);
        } else {
            out = forward_ngate(net, tests[i][0], tests[i][1]);
        }
        if (out == tests[i][2]) correct++;
    }
    return correct;
}

// Test OR
int test_or(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {1,1,1}};
    int correct = 0;
    
    for (int i = 0; i < 4; i++) {
        int out;
        if (net->num_gates == 1) {
            out = forward_1gate(net, tests[i][0], tests[i][1]);
        } else if (net->num_gates == 2) {
            out = forward_2gate(net, tests[i][0], tests[i][1]);
        } else {
            out = forward_ngate(net, tests[i][0], tests[i][1]);
        }
        if (out == tests[i][2]) correct++;
    }
    return correct;
}

// Test XOR
int test_xor(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {1,1,0}};
    int correct = 0;
    
    for (int i = 0; i < 4; i++) {
        int out;
        if (net->num_gates == 1) {
            out = forward_1gate(net, tests[i][0], tests[i][1]);
        } else if (net->num_gates == 2) {
            out = forward_2gate(net, tests[i][0], tests[i][1]);
        } else {
            out = forward_ngate(net, tests[i][0], tests[i][1]);
        }
        if (out == tests[i][2]) correct++;
    }
    return correct;
}

// Mutate network
void mutate(Network* net) {
    int choice = rand() % 3;
    
    switch(choice) {
        case 0: // Change gate type
            net->gates[rand() % net->num_gates].type = rand() % 4;
            break;
        case 1: // Change connection (if multiple gates)
            if (net->num_gates > 1) {
                int from = rand() % (net->num_gates - 1);
                int to = from + 1 + rand() % (net->num_gates - from - 1);
                net->connections[from][to] ^= 1;
            }
            break;
        case 2: // Reset memory
            net->gates[rand() % net->num_gates].memory = 0;
            break;
    }
}

// Test what N gates can solve
void test_n_gates(int n) {
    printf("\n=== Testing %d Gate%s ===\n", n, n > 1 ? "s" : "");
    
    // Test each problem
    typedef struct {
        char* name;
        int (*test_func)(Network*);
        int max_score;
    } Test;
    
    Test tests[] = {
        {"NOT", test_not, 2},
        {"AND", test_and, 4},
        {"OR", test_or, 4},
        {"XOR", test_xor, 4}
    };
    
    for (int t = 0; t < 4; t++) {
        Network* net = create_network(n);
        Network* best = create_network(n);
        int best_score = 0;
        
        // Random initialization
        for (int i = 0; i < n; i++) {
            net->gates[i].type = rand() % 4;
            net->gates[i].memory = 0;
        }
        
        // Evolution
        for (int gen = 0; gen < MAX_GENERATIONS; gen++) {
            int score = tests[t].test_func(net);
            
            if (score > best_score) {
                best_score = score;
                // Copy network
                for (int i = 0; i < n; i++) {
                    best->gates[i] = net->gates[i];
                }
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        best->connections[i][j] = net->connections[i][j];
                    }
                }
            }
            
            if (score == tests[t].max_score) {
                break;  // Solved
            }
            
            // Save and mutate
            Network* saved = create_network(n);
            for (int i = 0; i < n; i++) {
                saved->gates[i] = net->gates[i];
            }
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    saved->connections[i][j] = net->connections[i][j];
                }
            }
            
            mutate(net);
            
            if (tests[t].test_func(net) < score) {
                // Revert
                for (int i = 0; i < n; i++) {
                    net->gates[i] = saved->gates[i];
                }
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        net->connections[i][j] = saved->connections[i][j];
                    }
                }
            }
            
            free_network(saved);
        }
        
        // Report results
        printf("%s: %s (%d/%d)", tests[t].name, 
               best_score == tests[t].max_score ? "SOLVED" : "FAILED",
               best_score, tests[t].max_score);
        
        if (best_score == tests[t].max_score && n <= 3) {
            printf(" - Config: ");
            for (int i = 0; i < n; i++) {
                const char* types[] = {"PASS", "NOT", "MEM", "XOR_MEM"};
                printf("%s ", types[best->gates[i].type]);
            }
        }
        printf("\n");
        
        free_network(net);
        free_network(best);
    }
}

int main() {
    srand(time(NULL));
    
    printf("=== Systematic Gate Capability Test ===\n");
    printf("Testing what N gates can compute...\n");
    
    // Test 1 through 8 gates
    for (int n = 1; n <= 8; n++) {
        test_n_gates(n);
    }
    
    return 0;
}